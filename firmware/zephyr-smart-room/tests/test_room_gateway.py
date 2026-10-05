from tools.room_gateway import RoomState, parse_key_values, parse_state


def test_parse_key_values():
	assert parse_key_values("ROOM STATE occupied=1 light_on=0") == {
		"occupied": "1",
		"light_on": "0",
	}


def test_parse_state():
	assert parse_state("ROOM STATE occupied=1 light_on=1 contact_open=0 alarm=warning") == RoomState(
		occupied=1,
		light_on=1,
		contact_open=0,
		alarm="warning",
	)


def test_parse_state_rejects_non_state_line():
	try:
		parse_state("ROOM ACK property=light_on changed=1")
	except ValueError:
		return
	assert False, "expected ValueError"
