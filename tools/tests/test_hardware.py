"""Hardware smoke test; connect to the guest's forwarded TCP port.
The light is toggled and restored. Keep the Qt client closed for this test.
"""
import socket,time,sys
class Client:
    def __init__(self):
        self.sock=socket.create_connection(("127.0.0.1",5556),5)
        self.sock.settimeout(.3)
        self.buffer=b""
        self.lines=[]
    def send(self,line):self.sock.sendall((line+"\n").encode())
    def wait(self,check,seconds=8):
        end=time.monotonic()+seconds
        while time.monotonic()<end:
            if b"\n" not in self.buffer:
                try:
                    data=self.sock.recv(4096)
                    if not data:raise RuntimeError("Connection closed")
                    self.buffer+=data
                except socket.timeout:continue
            while b"\n" in self.buffer:
                raw,self.buffer=self.buffer.split(b"\n",1)
                line=raw.decode().strip()
                self.lines.append(line)
                print(line,flush=True)
                if check(line):return line
        raise RuntimeError("Timed out waiting for expected response")
    def close(self):self.sock.close()
c=Client()
try:
    state=c.wait(lambda s:s.startswith("ROOM STATE"))
    original="light_on=1" in state
    c.wait(lambda s:s.startswith("GATEWAY STATUS") and "ready=1" in s)
    for value in [1,0,int(original)]:
        c.send(f"SET light_on {value}")
        result=c.wait(lambda s:s.startswith("GATEWAY RESULT") and "status=pending" not in s)
        assert "status=confirmed" in result,result
        c.wait(lambda s:"GATEWAY STATUS" in s and "ready=1" in s)
    c.send("SET alarm 2")
    assert "invalid_command" in c.wait(lambda s:s.startswith("GATEWAY RESULT"))
finally:c.close()
time.sleep(.3)
c=Client()
try:
    c.wait(lambda s:s.startswith("ROOM STATE"))
    c.wait(lambda s:"GATEWAY STATUS" in s and "ready=1" in s)
finally:c.close()
print("PASS: real nRF -> QEMU ARM64 -> Windows TCP; light commands, whitelist and reconnect")
