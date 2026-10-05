#!/usr/bin/env python3
"""Exercise the real gateway executable through a pseudo-UART and TCP."""
import os,pty,socket,subprocess,sys,threading,time,select
master,slave=pty.openpty()
path=os.ttyname(slave)
probe=socket.socket();probe.bind(("127.0.0.1",0));port=probe.getsockname()[1];probe.close()
proc=subprocess.Popen([sys.argv[1],"--serial",path,"--port",str(port)],stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
stop=threading.Event()
mode="normal"
light=0
commands=[]
write_lock=threading.Lock()
def emit(line):
    data=(line+"\r\n").encode()
    with write_lock:
        os.write(master,data[:7]);time.sleep(.002);os.write(master,data[7:])
def state():
    return f"ROOM STATE occupied=0 light_on={light} contact_open=0 alarm=none"
def device():
    global light
    buf=b""
    while not stop.is_set():
        if not select.select([master],[],[],.1)[0]:continue
        buf+=os.read(master,4096)
        while b"\n" in buf:
            raw,buf=buf.split(b"\n",1)
            line=raw.decode().strip();commands.append(line)
            if mode=="silent":continue
            if line=="PING":emit("ROOM PONG")
            elif line=="GET STATE":emit(state())
            elif line.startswith("SET light_on") and mode=="normal":
                light=int(line[-1]);emit(state())
                emit("ROOM ACK property=light_on changed=1")
thread=threading.Thread(target=device,daemon=True);thread.start()
class Client:
    def __init__(self):
        deadline=time.monotonic()+5
        while True:
            try:self.s=socket.create_connection(("127.0.0.1",port),.3);break
            except OSError:
                if time.monotonic()>deadline:raise
                time.sleep(.05)
        self.s.settimeout(.2);self.buf=b""
    def send(self,s):self.s.sendall((s+"\n").encode())
    def wait(self,text,seconds=8):
        end=time.monotonic()+seconds
        while time.monotonic()<end:
            while b"\n" in self.buf:
                line,self.buf=self.buf.split(b"\n",1)
                if text in line.decode():return line.decode()
            try:
                data=self.s.recv(4096)
                if not data:raise AssertionError("Unexpected disconnect")
                self.buf+=data
            except socket.timeout:pass
        raise AssertionError("Missing: "+text)
    def close(self):self.s.close()
c=None
try:
    c=Client();c.wait("ready=1")
    c.s.sendall(b"SET light_");time.sleep(.03);c.s.sendall(b"on 1\r\n")
    c.wait("status=pending");c.wait("status=confirmed");c.wait("ready=1")
    other=Client();assert other.s.recv(1)==b"";other.close()
    c.send("SET alarm 2");c.wait("invalid_command")
    mode="ignore_set";before=commands.count("SET light_on 0")
    c.send("SET light_on 0");c.wait("status=pending");c.wait("status=timeout")
    assert commands.count("SET light_on 0")==before+1
    # Wait for a fresh response after the recovery quarantine.
    c.wait("ready=1",7)
    c.send("SET light_on 0");c.wait("status=pending")
    emit("ROOM BOOT proto=1 node=smart_room")
    c.wait("reason=device_reset");c.wait("ready=1")
    mode="silent";c.wait("online=0 valid=0",8)
    mode="normal";c.wait("online=1 valid=1",6)
    c.s.sendall(b"x"*1200+b"SET light_on 0\n")
    # Oversized client frames close the connection, never execute the suffix.
    time.sleep(.2);c.close();c=Client();c.wait("ready=1")
    print("PASS: real Asio IO, fragmented UART/TCP, single client, timeout, boot, heartbeat, oversized TCP")
finally:
    if c:c.close()
    stop.set();proc.terminate()
    try:proc.wait(timeout=3)
    except subprocess.TimeoutExpired:proc.kill();proc.wait()
    thread.join(1)
    os.close(master);os.close(slave)
    errors=proc.stderr.read().decode()
    if proc.returncode!=0:raise RuntimeError(errors)
