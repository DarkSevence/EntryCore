import localeInfo

IP_ADDRESS = 'TU_PODAJESZ_ADRES_IP'

def buildServerList(order_list):
	markAdressDict = {}
	authAdressDict = {}
	regionDict = {}
	region_index = 1

	for region, auth, mark, channels in order_list:
		channel_dict = {}
		for channel_index, channel in enumerate(channels, 1):
			key = region_index * 10 + channel_index
			channel['key'] = key
			channel_dict[channel_index] = channel

		region['channel'] = channel_dict
		regionDict[region_index] = region
		authAdressDict[region_index] = auth
		markAdressDict[region_index * 10] = mark
		region_index += 1

	return regionDict, authAdressDict, markAdressDict

if localeInfo.IsEUROPE():
	STATE_UNKNOWN = '...'
	STATE_DICT = {
		0: "....",
		1: "NORM",
		2: "BUSY",
		3: "FULL"
	}

	CHANNELS = [
		{'name': 'CH1', 'ip': IP_ADDRESS, 'tcp_port': 13000, 'udp_port': 13000, 'state': STATE_UNKNOWN},
		{'name': 'CH2', 'ip': IP_ADDRESS, 'tcp_port': 13001, 'udp_port': 13001, 'state': STATE_UNKNOWN},
		{'name': 'CH3', 'ip': IP_ADDRESS, 'tcp_port': 13002, 'udp_port': 13002, 'state': STATE_UNKNOWN},
		{'name': 'CH4', 'ip': IP_ADDRESS, 'tcp_port': 13003, 'udp_port': 13003, 'state': STATE_UNKNOWN},
	]
	
	MARK = {'ip': IP_ADDRESS, 'tcp_port': 13000, "mark" : "120.tga", "symbol_path" : "120",}
	
	AUTH = {'ip': IP_ADDRESS, 'port': 11002}
	NAME = {'name': 'EntryCore v1.9r'}

	REGION_ORDER_LIST = [(NAME, AUTH, MARK, CHANNELS)]
	NEW_REGION, NEW_AUTH_SERVER_DICT, NEW_MARK_ADDRESS_DICT = buildServerList(REGION_ORDER_LIST)
	NEW_REGION_NAME_DICT = {0: 'POLAND'}
	NEW_AUTH_SERVER_DICT = {0: NEW_AUTH_SERVER_DICT}
	NEW_REGION_DICT = {0: NEW_REGION}

	MARK_ADDRESS_DICT = NEW_MARK_ADDRESS_DICT
	REGION_DICT = NEW_REGION_DICT
	REGION_NAME_DICT = NEW_REGION_NAME_DICT
	AUTH_SERVER_DICT = NEW_AUTH_SERVER_DICT

	TEST_ADDRESS = {'ip': IP_ADDRESS, 'tcp_port': 50000, 'udp_port': 50000}
