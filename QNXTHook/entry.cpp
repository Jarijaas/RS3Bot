#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <stdio.h>

#include <capstone.h>
#include <x86.h>

#include <Windows.h>
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Ws2_32.lib")  // For yara
#pragma comment(lib, "Crypt32.lib") // For yara

#include "common.h"
#include "net.h"

#include <yara.h>

#include <function_hooks.h>

#include "Inventory.h"

#include <gl\GL.h>

#include "Matrix.h"

#include "Server.h"

#pragma comment(lib, "opengl32")

using namespace std;

uint64_t g_gameBase = 0;
std::unordered_map<DispatcherType, uint64_t> signatureByType;
std::unordered_map<DispatcherType, std::vector<uint64_t>> signaturesByType;

RPCServer server;

bool CreateConsole() {
	if (AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
		return true;
	}
	return false;
}

csh cs_handle;

bool InitializeCapstone() {
	if (cs_open(CS_ARCH_X86, CS_MODE_64, &cs_handle) == CS_ERR_OK) {
		cs_option(cs_handle, CS_OPT_DETAIL, CS_OPT_ON);
		cs_option(cs_handle, CS_OPT_SKIPDATA, CS_OPT_OFF);
		return true;
	}
	return false;
}

bool DeinitializeCapstone() { return cs_close(&cs_handle) == CS_ERR_OK; }

YR_RULES *yrRules;

bool InitializeYara() {
	if (yr_initialize() == ERROR_SUCCESS) {
		cout << "Yara initialized\n";
		if (yr_rules_load("yara-rules.yarc",
			&yrRules) == ERROR_SUCCESS) {
			cout << "Loaded yara rules\n";
			return true;
		}
		cout << "Could not load yar rules\n";
	}
	return false;
}

bool DeinitializeYara() { return yr_finalize() == ERROR_SUCCESS; }

uint64_t FindFunction(uint64_t addr) {
	cs_insn *insn;
	uint64_t fAddr = addr;
	size_t count = cs_disasm(cs_handle, (uint8_t *)addr, 0x10, addr, 0, &insn);
	if (count >= 2) {
		for (int i = 1; i < count; ++i) {
			// insn[i - 1].id == x86_insn::X86_INS_ADD &&
			if (insn[i].id == x86_insn::X86_INS_JMP) {
				fAddr = X86_REL_ADDR(insn[i]);
				break;
			}
		}
	}
	cs_free(insn, count);
	return fAddr;
}

uint64_t g_opengl = 0;

void CALLBACK LowLevelMouseProc(int code, WPARAM param, LPARAM lParam) {
	// int xcode = code;

	if (param == VK_LBUTTON) {
		cout << "left click\n";
	}
	cout << "param: " << param;
	// cout << "code: " << xcode << endl;
	// cout << "Hook callback!!!\n";
}

void __fastcall OnSendData(int x) { cout << "Sent: " << x << endl; }

void SetSlotItem(uint8_t *msgData, size_t msgSize) {
	// slotLocation = 0
	// quantity = 6
	// slotIndex = 3
	// itemId = 4, (>> 8) - 1

	uint8_t offset = 0;

	bool unk1 = (msgData[2] >> 1) & 1;
	bool unk2 = msgData[2] & 1;

	uint32_t actId =
		static_cast<uint32_t>(_rotl16(*reinterpret_cast<uint16_t *>(msgData), 8));

	offset += 3;

	// cout << "actId: " << dec << actId << endl;
	// cout << "invPtr:" << hex <<
	// Game::GetInventoryByType(Game::InventoryType::CoinPouch) << endl;

	// cout << "bagpack begin: " << hex << bagpack.begin() << endl;

	/*cout << "msgSize: " << dec << msgSize << endl;*/

	uint16_t slot = msgData[offset];
	if (slot >= 0x80) {
		slot = _rotl16(*reinterpret_cast<uint16_t *>(msgData + offset), 8) + 0x8000;
		offset += 2;
	}
	else {
		offset += 1;
	}

	uint16_t itemId = _rotl16(*reinterpret_cast<uint16_t *>(msgData + offset), 8);
	offset += 2;

	uint8_t count = 0;

	if (itemId != 0) {
		uint32_t quantity = *reinterpret_cast<uint32_t *>(msgData + offset) & 0xFF;

		if (quantity == 0xFF) {
			quantity = *reinterpret_cast<uint32_t *>(msgData + offset);
			uint32_t t = quantity << 8;
			quantity = ((quantity >> 8) ^ (quantity << 8)) & 0x0FF00FF;
			quantity = _rotl(quantity ^ t, 16);
			offset += 5;
		}
		else {
			offset += 1;
		}

		if (unk1) {
			count = msgData[offset];
			offset += 1;
		}
	}
	// cout << "itemId: " << dec << itemId << ", slot: " << slot << ", count: " <<
	// (int)count << endl;

	for (int i = 0; i < count; ++i) {
		uint8_t unk2 = _rotl16(*reinterpret_cast<uint16_t *>(msgData + offset), 8);
		offset += 2;
		uint32_t unk3 = *reinterpret_cast<uint32_t *>(msgData + offset);
		/*uint32_t t = ((unk3 >> 8) ^ (unk3 << 8)) & 0x0FF00FF;
		unk3 = ((t ^ unk3 << 8) >> 16) | ((t ^ unk3 << 8) << 16);*/
		unk3 = (((unk3 << 8) ^ ((unk3 << 8) ^ (unk3 >> 8)) & 0xFF00FF) << 16) |
			(unsigned __int16)(((unk3 << 8) ^
			((unk3 << 8) ^ (unk3 >> 8)) & 0xFF00FFu) >>
				16);

		offset += 4;
	}

	/*uint16_t unk2 = *reinterpret_cast<uint32_t*>(msgData + offset + 7);
	offset += 2;
	uint32_t unk3 = *reinterpret_cast<uint32_t*>(msgData + offset + 7);

	uint32_t t = ((unk3 >> 8) ^ (unk3 << 8)) & 0x0FF00FF;
	unk3 = ((t ^ unk3 << 8) >> 16) | ((t ^ unk3 << 8) << 16);

	offset += 4;
	cout << "SetItem: " << dec << "unk1:" << (int)unk1 << ", slot: " <<  (int)slot
	<< ", " << "itemId: " <<  (short)itemId << ", actId: " << (int)actId << ",
	quantity: " << (int)quantity << ", unk2:" << (int)unk2 << ", unk3:" <<
	(int)unk3 << endl;*/
}

void HandleRecvGameMessage(uint8_t *msgData) {
	uint32_t index = 0;
	uint16_t type = msgData[index];
	if (type >= 0x80) {
		type = _rotl16(*reinterpret_cast<uint16_t *>(msgData), 8) + 0x8000;
		index += 2;
	}
	else {
		index++;
	}

	uint32_t unk2 = *reinterpret_cast<uint32_t *>(&msgData[index]);
	unk2 = ((unk2 >> 8) ^ (unk2 << 8)) & 0x0FF00FF;
	unk2 = _rotl(unk2 ^ (unk2 << 8), 16);
	index += 4;

	uint8_t unk3 = msgData[++index];

	ChatMessage msg;
	msg.set_content(reinterpret_cast<char *>(&msgData[index]));
	msg.set_type(type);
	server.PushEvent(msg);
}

uint32_t counter1 = 0;
uint32_t counter2 = 0;

void HandleUpdateObject(uint8_t *msgData, size_t msgSize) {
	counter1 += 8 * -msgData[0];
	counter2 += msgData[1];
	uint8_t baseId2 = msgData[2];
	return;

	// cout << dec << (int)byte1 << ", " << (int)byte2 << ", " << (int)byte3 <<
	// endl;

	for (int i = 3; i < msgSize; i += 7) {
		uint8_t type = msgData[i];
		// cout << "type: " << (int)type << endl;
		if (type > 14) {
			break;
		}
		// cout << "type: " << (int)type << endl;

		// Update object
		if (type == 8) {
			uint16_t id = msgData[i + 1] << 8 | msgData[i + 2];
			uint32_t objData = *reinterpret_cast<uint32_t *>(&msgData[i + 3]);

			uint16_t modelId = *reinterpret_cast<uint16_t *>(&msgData[i + 5]);
			// msgData[i + 6] = 0x5;
			// msgData[i + 5] = 0xC3;
			uint32_t newObjData = *reinterpret_cast<uint32_t *>(&msgData[i + 3]);

			cout << "Object update by id: " << dec << (int)id << ", " << (id & 0xFF)
				<< ", data: " << hex << objData << ", modelId: " << dec << modelId
				<< endl;
			// uint32_t unk1 = *reinterpret_cast<uint32_t*>(&msgData[i + 3]);
			// cout << "b5: " << (int)byte5 << ", b6: " << (int)byte6 << ", unk1: " <<
			// hex << unk1 << endl;
		}
		else if (type == 9) {
			uint16_t id = msgData[i + 1] << 8 | msgData[i + 2];
			// Tree cutdown
			cout << "Tree cutdown id: " << dec << (int)id << ", " << (id & 0xFF)
				<< endl;
		}
		/*else if (type == 10) {
				cout << "Tree remove trunk: " << dec << id << endl;
		}
		else if (type == 5) {
				cout << "Tree restored5: " << dec << id << endl;
		}
		else if (type == 12) {
				cout << "Tree restored3: " << dec << id << endl;
		}
		else if (type > 1) {
				/*cout << "type: " << (int)type << endl;
				cout << "Id: " << dec << (int)id << endl;*/
				/*}*/
	}

	/*cout << "UpdateObject: ";
	for (int i = 0; i < 8; ++i) {
			cout << hex << (int)msgData[i] << "h ";
	}
	cout << endl;*/
}

void HandleSetSkillXP(uint8_t *msgData) {
	// uint8_t unk1 = msgData[0] + 0x80;
	uint8_t skillIndex = -msgData[1];
	uint32_t skillXP = *reinterpret_cast<uint32_t *>(&msgData[2]);
	// cout << "SetSkillXP: " << dec << (int)skillId << ", " << (int)skillIndex <<
	// ", " << skillXP << endl;
}

void HandleSetAccountSecuritySettings(uint8_t *msgData) {
	// cout << "SetAccountSecuritySettings\n";
}

void HandleSetGameState(uint8_t *msgData) {
	uint16_t state = _rotl16(*reinterpret_cast<uint16_t *>(&msgData[4]), 8);
	cout << "SetGameState: " << dec << state << endl;
}

void HandleSetLobbyNews(uint8_t *msgData) {
	// cout << "SetLobbyNews\n";
}

void HandleSetGameData(uint8_t *msgData) {
	// cout << "SetGameData\n";
}

void HandleSetWorlds(uint8_t *msgData) {
	// cout << "SetWorlds\n";
}

void UnitTests_Unpack() {
	uint8_t data1[] = { 0x1E, 0x00, 0x5E, 0xB8, 0x50, 0x0A, 0xD5, 0x29 };

	uint32_t currIdx = 0;
	if (UnpackData<uint8_t>(data1, 2, 18, &currIdx) != 1) {
		cout << "Unpack unit test #1 failed\n";
	}

	if (UnpackData<uint8_t>(data1, 2, 25, &currIdx) != 1) {
		cout << "Unpack unit test #2 failed\n";
	}
}

uint32_t UnpackTile(uint8_t *data, uint32_t bitIndex, uint32_t *outBitIndex) {
	return UnpackData<uint32_t>(data, 0x1E, bitIndex, outBitIndex);
}

void HandleLoadWorld(uint8_t *msgData) {
	cout << "Loaded region\n";

	uint32_t bitIndex = 0;

	uint32_t tileId = UnpackTile(msgData, bitIndex, &bitIndex);

	uint16_t x = (tileId >> 14) & 0x3FFF;
	uint16_t y = tileId & 0x3FFF;

	cout << "Location: " << hex << tileId << ", x:" << dec << x << ", y: " << y
		<< endl;
}

void HandleUpdateNPC(uint8_t *msgData, int msgSize) {
	return;
	int offset = 0;

	uint32_t bitIndex = 0;

	uint32_t count = UnpackData<uint32_t>(msgData, 8, bitIndex, &bitIndex);

	for (uint32_t i = 0; i < count; ++i) {
		uint8_t needsUpdate = UnpackData<uint8_t>(msgData, 1, bitIndex, &bitIndex);

		if (needsUpdate) {
			uint8_t state = UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);
			if (state == 0) {
				// cout << "idle\n";
			}
			else if (state == 1) {
				uint8_t primaryDirection =
					UnpackData<uint8_t>(msgData, 3, bitIndex, &bitIndex);
				// cout << "primaryDirection: " << (int)primaryDirection << endl;
				if (UnpackData<uint8_t>(msgData, 1, bitIndex, &bitIndex) == 1) {
					// cout << "update\n";
				}
			}
			else if (state == 2) {
				uint8_t mask1 = 0, mask2 = 0;
				if (UnpackData<uint8_t>(msgData, 1, bitIndex, &bitIndex) == 1) {
					uint8_t mask2 = UnpackData<uint8_t>(msgData, 3, bitIndex, &bitIndex);
				}
				mask1 = UnpackData<uint8_t>(msgData, 3, bitIndex, &bitIndex);
				uint8_t mask = mask2 << 3 | mask1;

				if (UnpackData<uint8_t>(msgData, 1, bitIndex, &bitIndex) == 1) {
					// cout << "update\n";
				}
			}
			else if (state == 3) {
				// cout << "remove\n";
			}
		}
	}

	int msgSizeBits = 8 * msgSize;

	cout << "msgSize: " << (int)(msgSize) << endl;
	while ((int)(msgSizeBits - bitIndex) >= 15) {
		cout << (msgSizeBits - bitIndex) << endl;

		uint16_t npcId = UnpackData<uint16_t>(msgData, 15, bitIndex, &bitIndex);
		if (npcId == 0x7FFF) {
			break;
		}
		cout << "npcId: " << dec << (int)npcId << endl;

		uint8_t slot1 = UnpackData<uint8_t>(msgData, 7, bitIndex, &bitIndex);
		uint8_t slot2 = UnpackData<uint8_t>(msgData, 7, bitIndex, &bitIndex);
		uint16_t deltaPos = UnpackData<uint8_t>(msgData, 15, bitIndex, &bitIndex);

		uint8_t unk3 = UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);
		uint8_t updatePos = UnpackData<uint8_t>(msgData, 1, bitIndex, &bitIndex);
		uint8_t unk5 = UnpackData<uint8_t>(msgData, 1, bitIndex, &bitIndex);
		uint8_t posu = UnpackData<uint8_t>(msgData, 3, bitIndex, &bitIndex);
	}

	// cout << "Update NPC. size: " << hex << bitIndex << ", " << (bitIndex >> 3)
	// << endl;

	/*uint8_t unk2 = UnpackData<uint8_t>(msgData, 1, bitIndex, &bitIndex);

	uint8_t unk3 = UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);

	cout << "Update NPC. index: "  << unk1 << ", unk2: " << (int)unk2 << ", unk3:
	" << (int)unk3 << endl;*/
}

int pCount1 = 0;
int pCount2 = 0;

uint8_t UpdatePlayerStuff(uint8_t *msgData, uint32_t *bitIndex) {
	uint8_t unk1 = UnpackData<uint8_t>(msgData, 2, *bitIndex, bitIndex);

	if (unk1 != 0) {
		if (unk1 == 1) {
			uint8_t unk2 = UnpackData<uint8_t>(msgData, 2, *bitIndex, bitIndex);
		}
		else if (unk1 != 2) {
			uint32_t unk2 = UnpackData<uint8_t>(msgData, 20, *bitIndex, bitIndex);
		}
		else {
			uint8_t unk2 = UnpackData<uint8_t>(msgData, 5, *bitIndex, bitIndex);
		}
		return 0;
	}
	uint8_t unk2 = UnpackData<uint8_t>(msgData, 1, *bitIndex, bitIndex);
	if (unk2 == 1) {
		UpdatePlayerStuff(msgData, bitIndex);
	}
	uint8_t unk3 = UnpackData<uint8_t>(msgData, 6, *bitIndex, bitIndex);
	uint8_t unk4 = UnpackData<uint8_t>(msgData, 6, *bitIndex, bitIndex);
	uint8_t unk5 = UnpackData<uint8_t>(msgData, 1, *bitIndex, bitIndex);
	return 1;
}

void UpdatePlayerMovement(uint8_t *msgData, uint32_t *bitIndex) {
	uint8_t unk1 = UnpackData<uint8_t>(msgData, 1, *bitIndex, bitIndex);
	uint8_t unk2 = UnpackData<uint8_t>(msgData, 2, *bitIndex, bitIndex);
	if (unk2 == 0) {
		if (unk1 != 0) {
			return;
		}

		uint32_t unk3 = UnpackData<uint8_t>(msgData, 1, *bitIndex, bitIndex);
		if (unk3 == 1) {
			unk3 = UpdatePlayerStuff(msgData, bitIndex);
		}
		return;
	}

	if (unk2 == 1) {
		uint8_t direction = UnpackData<uint8_t>(msgData, 3, *bitIndex, bitIndex);
		uint8_t unk3 = UnpackData<uint8_t>(msgData, 1, *bitIndex, bitIndex);
		uint8_t unk4 = 0;

		if (unk3 == 1) {
			unk4 = UnpackData<uint8_t>(msgData, 1, *bitIndex, bitIndex);
		}
	}
	else if (unk2 == 2) {
		uint8_t direction2 = UnpackData<uint8_t>(msgData, 4, *bitIndex, bitIndex);
	}
	else {
		uint8_t unk3 = UnpackData<uint8_t>(msgData, 1, *bitIndex, bitIndex);
		if (unk3 == 1) {
			uint8_t id = UnpackData<uint8_t>(msgData, 3, *bitIndex, bitIndex);
		}
		else {
			uint8_t id = UnpackData<uint8_t>(msgData, 15, *bitIndex, bitIndex);
		}
	}
}

void HandleUpdatePlayers(uint8_t *msgData) {
	return;
	// cout << "UpdatePlayers: " << pCount1 << ", " << pCount2 << endl;
	uint32_t bitIndex = 0;

	uint32_t newPCount1 = pCount1;
	uint32_t newPCount2 = pCount2;

	uint32_t playerCount = 0;
	for (int i = 0; i < pCount1 + 1; ++i) {
		if (playerCount > 0) {
			playerCount--;
		}

		bool updatePlayer = UnpackData<bool>(msgData, 1, bitIndex, &bitIndex);
		if (updatePlayer) {
			bool isAnimating = UnpackData<bool>(msgData, 1, bitIndex, &bitIndex);

			uint8_t movementType =
				UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);
			if (movementType == 0) {
				// cout << "Nothing to update\n";
			}
			else if (movementType == 1) {
				uint8_t direction =
					UnpackData<uint8_t>(msgData, 3, bitIndex, &bitIndex);
				bool updateRequired = UnpackData<bool>(msgData, 1, bitIndex, &bitIndex);
				uint8_t unk2 = 0;
				if (updateRequired) {
					unk2 = UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);
				}
				cout << "Walk in direction: " << (int)direction << endl;
			}
			else if (movementType == 2) {
				uint8_t direction =
					UnpackData<uint8_t>(msgData, 3, bitIndex, &bitIndex);
				bool updateRequired = UnpackData<bool>(msgData, 1, bitIndex, &bitIndex);
				cout << "Ran in direction: " << (int)direction << endl;
			}
			else if (movementType == 3) {
				// update plane
				uint8_t unk2 = UnpackData<uint8_t>(msgData, 1, bitIndex, &bitIndex);
				uint32_t pos = 0;
				if (unk2 == 1) {
					pos = UnpackData<uint8_t>(msgData, 30, bitIndex, &bitIndex);
				}
				else {
					pos = UnpackData<uint8_t>(msgData, 15, bitIndex, &bitIndex);
				}
				// cout << "unk2: " <<  (int)unk2 << ", pos: " << pos << endl;
			}
		}
		else {
			uint8_t unk1 = UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);
			if (unk1 != 0) {
				if (unk1 == 1) {
					playerCount = UnpackData<uint16_t>(msgData, 5, bitIndex, &bitIndex);
				}
				else if (unk1 == 2) {
					playerCount = UnpackData<uint16_t>(msgData, 8, bitIndex, &bitIndex);
				}
				else {
					playerCount = UnpackData<uint16_t>(msgData, 11, bitIndex, &bitIndex);
				}
			}
			cout << "playerCount: " << playerCount << endl;
			pCount1 = playerCount;
		}
	}

	return;
	int v3 = 0;

	if (!v3) {
		for (int i = 0; i < pCount1; ++i) {
			if (v3) {
				--v3;
				newPCount1++;
				continue;
			}

			bool needsPlacement = UnpackData<bool>(msgData, 1, bitIndex, &bitIndex);
			if (needsPlacement) {
				cout << "Update P: " << i << endl;
				UpdatePlayerMovement(msgData, &bitIndex);
			}
			else {
				uint8_t unk1 = UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);
				if (unk1 != 0) {
					if (unk1 == 1) {
						v3 = UnpackData<uint16_t>(msgData, 5, bitIndex, &bitIndex);
					}
					else if (unk1 == 2) {
						v3 = UnpackData<uint16_t>(msgData, 8, bitIndex, &bitIndex);
					}
					else {
						v3 = UnpackData<uint16_t>(msgData, 11, bitIndex, &bitIndex);
					}
				}
				else {
					v3 = 0;
				}
				cout << "v3_1:" << v3 << endl;
			}
		}
	}

	if (!v3) {
		for (int i = 0; i < pCount1; ++i) {
			if (v3) {
				--v3;
				newPCount1++;
				continue;
			}

			bool needsPlacement = UnpackData<bool>(msgData, 1, bitIndex, &bitIndex);
			if (needsPlacement) {
				cout << "Update P: " << i << endl;
				UpdatePlayerMovement(msgData, &bitIndex);
			}
			else {
				uint8_t unk1 = UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);
				if (unk1 != 0) {
					if (unk1 == 1) {
						v3 = UnpackData<uint16_t>(msgData, 5, bitIndex, &bitIndex);
					}
					else if (unk1 == 2) {
						v3 = UnpackData<uint16_t>(msgData, 8, bitIndex, &bitIndex);
					}
					else {
						v3 = UnpackData<uint16_t>(msgData, 11, bitIndex, &bitIndex);
					}
				}
				else {
					v3 = 0;
				}
				cout << "v3_2:" << v3 << endl;
			}
		}
	}

	if (!v3) {
		for (int i = 0; i < pCount2; ++i) {
			if (v3) {
				--v3;
				newPCount2++;
				continue;
			}

			bool needsPlacement = UnpackData<bool>(msgData, 1, bitIndex, &bitIndex);
			if (needsPlacement) {
				cout << "Update P: " << i << endl;
				UpdatePlayerStuff(msgData, &bitIndex);
			}
			else {
				uint8_t unk1 = UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);
				if (unk1 != 0) {
					if (unk1 == 1) {
						v3 = UnpackData<uint16_t>(msgData, 5, bitIndex, &bitIndex);
					}
					else if (unk1 == 2) {
						v3 = UnpackData<uint16_t>(msgData, 8, bitIndex, &bitIndex);
					}
					else {
						v3 = UnpackData<uint16_t>(msgData, 11, bitIndex, &bitIndex);
					}
				}
				else {
					v3 = 0;
				}
				cout << "v3_3:" << v3 << endl;
			}
		}
	}

	if (!v3) {
		for (int i = 0; i < pCount2; ++i) {
			if (v3) {
				--v3;
				newPCount2++;
				continue;
			}

			bool needsPlacement = UnpackData<bool>(msgData, 1, bitIndex, &bitIndex);
			if (needsPlacement) {
				cout << "Update P: " << i << endl;
				UpdatePlayerStuff(msgData, &bitIndex);
			}
			else {
				uint8_t unk1 = UnpackData<uint8_t>(msgData, 2, bitIndex, &bitIndex);
				if (unk1 != 0) {
					if (unk1 == 1) {
						v3 = UnpackData<uint16_t>(msgData, 5, bitIndex, &bitIndex);
					}
					else if (unk1 == 2) {
						v3 = UnpackData<uint16_t>(msgData, 8, bitIndex, &bitIndex);
					}
					else {
						v3 = UnpackData<uint16_t>(msgData, 11, bitIndex, &bitIndex);
					}
				}
				else {
					v3 = 0;
				}
				cout << "v3_4:" << v3 << endl;
			}
		}
	}

	pCount1 = newPCount1;
	pCount2 = newPCount2;
}

void HandleSetPlayerHP(uint64_t _this, uint8_t *msgData) {
	uint64_t parent = *reinterpret_cast<uint64_t *>(_this + 8);

	uint16_t id = (msgData[0] << 8) + ((msgData[1] - 128) & 0xFF);
	uint32_t val = *reinterpret_cast<uint32_t *>(&msgData[2]);

	val = (((val << 8) ^ ((val << 8) ^ (val >> 8)) & 0xFF00FF) << 16) |
		(((val << 8) ^ ((val << 8) ^ (val >> 8)) & 0xFF00FFu) >> 16) & 0xFFFF;
	// cout << "ID: " << dec << id << ", val: " << hex << val << endl;
}

void HandleOpenDialog(uint8_t *msgData) {
	// IDA ServerUICallback
	const uint32_t unswapped = *reinterpret_cast<uint32_t*>(&msgData[10]);
	const uint32_t id = _byteswap_ulong(unswapped);

	cout << "Interface ID: " << hex << id << endl;

	InterfaceOpenMessage msg;
	msg.set_interfaceid(id);
	server.PushEvent(msg);
}

std::unordered_map<uint64_t, uint16_t> signs;
std::unordered_map<uint16_t, uint64_t> idMapping;

/**
		NetGameRecvDispatch hook callback. Dispatches received message to its
   dispatcher.
		@param _this this pointer (RCX register)
		@param RDX RDX register
		@param R8 R8 register
		@param R9 R9 register
*/
void __fastcall OnDispatchNetMessage(fn_hooks::Registers regs) {
	uint64_t instance = *reinterpret_cast<uint64_t *>(regs.R8 + 0x40);
	uint64_t vtable = *reinterpret_cast<uint64_t *>(instance);
	uint32_t msgId = *reinterpret_cast<uint64_t *>(
		regs.R9 + 0x28); // dispatcher index in array
	uint64_t msg = regs.R9 + 0x2C8;
	uint8_t *msgData = *reinterpret_cast<uint8_t **>(msg + 0x10);
	uint64_t msgSize = regs.RDX;

	// disasm(dispatchFunc, 0x100);

	if (idMapping.find(msgId) == idMapping.end()) {
		uint64_t dispatchFunc =
			FindFunction(*reinterpret_cast<uint64_t *>(vtable + 0x10));
		uint64_t func =
			signs.find(dispatchFunc) != signs.end() ? signs[dispatchFunc] : NULL;
		idMapping[msgId] = func;
		if (func == NULL) {
			cout << "msgId: " << msgId << ", dispatchFunc: " << hex << dispatchFunc
				<< endl;
		}
	}

	switch (static_cast<DispatcherType>(idMapping[msgId])) {
	case DispatcherType::SetGameData:
		HandleSetGameData(msgData);
		break;
	case DispatcherType::SetLobbyNews:
		HandleSetLobbyNews(msgData);
		break;
	case DispatcherType::SetWorlds:
		HandleSetWorlds(msgData);
		break;
	case DispatcherType::GameMessage:
		HandleRecvGameMessage(msgData);
		break;
	case DispatcherType::UpdateObject:
		HandleUpdateObject(msgData, msgSize);
		break;
	case DispatcherType::SetSkillXP:
		HandleSetSkillXP(msgData);
		break;
	case DispatcherType::SetAccountSecuritySettings:
		HandleSetAccountSecuritySettings(msgData);
		break;
	case DispatcherType::SetGameState:
		HandleSetGameState(msgData);
		break;
	case DispatcherType::UpdateNPC:
		HandleUpdateNPC(msgData, msgSize);
		break;
	case DispatcherType::LoadWorld:
		HandleLoadWorld(msgData);
		break;
	case DispatcherType::SetInventorySlot:
		SetSlotItem(msgData, msgSize);
		break;
	case DispatcherType::UpdatePlayers:
		HandleUpdatePlayers(msgData);
		break;
	case DispatcherType::SetPlayerHP:
		HandleSetPlayerHP(instance, msgData);
		break;
	case DispatcherType::OpenDialog:
		HandleOpenDialog(msgData);
		break;
	default:
		// cout << "Unknown msgId: " << msgId << ", dispatchFunc: " << hex <<
		// dispatchFunc << endl;
		break;
	}
}

/*enum class GLenum : int {
		GL_DEPTH_COMPONENT = 0x1902,
		GL_DEPTH_STENCIL = 0x84F9,
		GL_FLOAT = 0x1406
};*/

typedef PROC(WINAPI *__wglGetProcAddress)(LPCSTR lpszProc);
__wglGetProcAddress _wglGetProcAddress;

void glReadPixels(int x, int y, uint32_t width, uint32_t height, GLenum format,
	GLenum type, void *data) {
	void *f = GetProcAddress((HMODULE)g_opengl, "glReadPixels");
	typedef void(__cdecl * _glReadPixels)(int, int, uint32_t, uint32_t, GLenum,
		GLenum, void *);
	reinterpret_cast<_glReadPixels>(f)(x, y, width, height, format, type, data);
}

/*int glGetError() {
		void *f = GetProcAddress((HMODULE)g_opengl, "glGetError");
		typedef int(__cdecl *_glGetError)();
		return reinterpret_cast<_glGetError>(f)();
}*/

int glGetUniformLocation(int program, const char *name) {
	// void *f = GetProcAddress((HMODULE)g_opengl, "glGetUniformLocation");
	// void *f = _wglGetProcAddress("glGetUniformLocation");
	// cout << "glGetUniformLocation: " << hex << (int)f << endl;
	void *f = *(void **)(g_gameBase + 0x69EAC0);

	typedef int(__cdecl * _glGetUniformLocation)(int, const char *);
	return reinterpret_cast<_glGetUniformLocation>(f)(program, name);
}

bool glIsProgram(int program) {
	void *f = *(void **)(g_gameBase + 0x69EB30);

	typedef bool(__cdecl * _glIsProgram)(int);
	return reinterpret_cast<_glIsProgram>(f)(program);
}

#include <fstream>
#include <iostream>

std::ofstream ofs("C:\\ProgramData\\Jagex\\uninames.txt");

void glGetUniformLocationHook(uint64_t _this, uint64_t data) {
	int program = *(int *)(_this + 0x3A78);

	const char *name = *(const char **)(data);

	ofs << "progmam: " << program << ", name: " << name << endl;
	// cout << "progmam: " << program << ", name: " << name << endl;
}

void glUniformMatrix4fv(int location, int count, bool transpose,
	const float *value) {
	void *f = _wglGetProcAddress("glUniformMatrix4fv");
	typedef void(__cdecl * _glUniformMatrix4fv)(int, int, bool, const float *);
	reinterpret_cast<_glUniformMatrix4fv>(f)(location, count, transpose, value);
}

void glGetUniformfv(int program, int location, float *params) {
	void *f = _wglGetProcAddress("glGetUniformfv");
	typedef void(__cdecl * _glGetUniformfv)(int, int, float *);
	reinterpret_cast<_glGetUniformfv>(f)(program, location, params);
}

void glUseProgram(int program) {
	void *f = _wglGetProcAddress("glUseProgram");
	typedef void(__cdecl * _glUseProgram)(int);
	reinterpret_cast<_glUseProgram>(f)(program);
}

#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_PROJECTION_MATRIX 0x0BA7
#define GL_VIEWPORT 0x0BA2

int counter = 0;

void __fastcall OnRender(uint64_t _this) {
	if (counter == 240) {
		counter = 0;

		int prevProg = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);

		float projMat[16] = { 0.0f };
		float viewPort[4];

		glGetFloatv(GL_VIEWPORT, viewPort);
		// glUseProgram(0x0d);

		/*int viewProjMatLoc = glGetUniformLocation(0x0d, "uViewProjMat");
		cout << "uViewProjMat: " << hex << viewProjMatLoc << endl;
		cout << "isProgram: " << glIsProgram(0x0d) << endl;*/

		int viewProjMatLoc = glGetUniformLocation(101, "uProjectionMatrix");
		cout << "uViewProjMat: " << hex << viewProjMatLoc << endl;
		cout << "isProgram: " << glIsProgram(101) << endl;

		float mat[4 * 4] = { 0.0f };
		glGetUniformfv(101, viewProjMatLoc, mat);
		// glUniformMatrix4fv(viewProjMatLoc, 1, true, mat);

		for (int j = 0; j < 4 * 4; ++j) {
			cout << projMat[j] << " ";
			if (((j + 1) % 4) == 0) {
				cout << endl;
			}
		}

		// lUseProgram(prevProg);
	}

	counter++;

	/* float *outData = new float[600 * 600];
	ZeroMemory(outData, 600 * 600 * sizeof(float));

	glReadPixels(200, 200, 600, 600, GLenum::GL_DEPTH_COMPONENT, GLenum::GL_FLOAT,
	outData); for (int x = 0; x < 600 * 600; ++x) { if (outData[x] != 0) { cout <<
	x << ": " << outData << endl;
			}
	}
	delete[] outData;
	cout << "glErr: " << hex << glGetError() << endl; */

	/*float *viewMatrix = reinterpret_cast<float*>(_this + 0x30);
	float *invViewProjMatrix = reinterpret_cast<float*>(_this + 0x70);
	float *cameraPos = reinterpret_cast<float*>(_this + 0xC0);

	float viewProjMatrix[16] = { 0 };
	gluInvertMatrix(invViewProjMatrix, viewProjMatrix);

	// float coords[] = { 3309, 3165, 1, 1};
	float screenP[] = { 0.5, 0.5, 0, 0 };
	float screenPT[4] = { 0 };
	float coordsT[16] = {0};
	float out[16] = { 0 };
	// MatrixTranspose(coords, 1, 4, coordsT);
	MatrixTranspose(screenP, 1, 4, screenPT);*/

	// MatrixMul(viewProjMatrix, 4, 4, coordsT, 4, 1, screenCoords);
	/* MatrixMul(invViewProjMatrix, 4, 4, screenPT, 4, 1, out);

	for (int j = 0; j < 4 * 4; ++j) {
			cout << out[j] << " ";
			if (((j + 1) % 4) == 0) {
					cout << endl;
			}
	}*/
	// cout << "x: " << cameraPos[0] << ", y " << cameraPos[1] << ", " <<
	// cameraPos[2] << endl;

	// cout << endl;
	// cout << endl;
}

typedef float(__cdecl *_TileProjection)(uint64_t _this, uint64_t camera,
	float *p1, float *p2, uint32_t iters);
typedef void(__cdecl *_MouseAction)(uint64_t _this, const char *text1,
	const char *text2, int unk2,
	uint64_t actionHandler, int unk4, int unk5,
	int tileX, int tileY, uint64_t unk8,
	uint64_t unk9, int unk10, uint64_t unk11,
	uint8_t unk12, uint64_t unk13,
	uint8_t unk14);

int cn = 0;

uint64_t g_entityList = 0;
uint64_t g_entityListFull = 0;
uint64_t *g_contextStore = nullptr;
uint64_t g_gameContext = 0;

bool initialized = false;

void __fastcall OnCursorWorldContextMenu(fn_hooks::Registers regs) {
	uint64_t _this = regs.RCX;

	try {
		if (!initialized) {
			server.Run();
			initialized = true;
		}

		g_gameContext = _this;
		uint64_t ptr1 = *(uint64_t *)(_this + 8);
		uint64_t ptr2 = *(uint64_t *)(ptr1 + 0x1160);
		uint64_t ptr3 = *(uint64_t *)(ptr2 + 0x70);
		uint64_t camera = *(uint64_t *)(ptr3 + 0x8);
		if (camera == 0) {
			return;
		}

		// cout << "camera: " << hex << camera << endl;

		// g_entityList = camera + 0xDA130;
		g_entityList = camera + 0xDA0B0;

		// mov     r8, [rbx+100B0h]
		// call    AddVisibleEntityCursor
		g_entityListFull = *(uint64_t *)(camera + 0x100B0);

		g_contextStore = *(uint64_t **)(*(uint64_t *)(ptr1 + 0x450) + 0x80);

		server.Handle();
	}
	catch (InvalidPointerException &ex) {
		cout << "Invalid pointer: " << ex.what() << endl;
	}

	return;
	/*float *projMatrix = (float*)(camera + 0x11AB0);
	float *viewMatrix = (float*)(camera + 0x11A70);

	float viewProj[4 * 4];
	float viewProjInv[4 * 4];

	MatrixMul(viewMatrix, 4, 4, projMatrix, 4, 4, viewProj);
	gluInvertMatrix(viewProj, viewProjInv);*/

	/*for (int j = 0; j < 4 * 4; ++j) {
			cout << viewProjInv[j] << " ";
			if (((j + 1) % 4) == 0) {
					cout << endl;
			}
	}*/

	/*POINT mousePos;
	GetCursorPos(&mousePos);
	HWND parHwnd = FindWindow("JagWindow", NULL);
	ScreenToClient(parHwnd, &mousePos);

	float in[4];

	RECT rect = { 0 };
	GetWindowRect(parHwnd, &rect);


	LONG resX = rect.right - rect.left;
	LONG resY = rect.bottom - rect.top;

	in[0] = (2.0f*mousePos.x / (float)resX) - 1.0f;
	in[1] = (2.0f*(resY - mousePos.y) / (float)resY) - 1.0f;
	in[2] = 1.0;
	in[3] = 1.0;

	float pos1[4] = { 0 };

	MatrixMul(in, 1, 4, viewProjInv, 4, 4, pos1);
	pos1[3] = 1 / pos1[3];

	pos1[0] *= pos1[3];
	pos1[1] *= pos1[3];
	pos1[2] *= pos1[3];

	in[0] = (2.0f*mousePos.x / (float)resX) - 1.0f;
	in[1] = (2.0f*(resY - mousePos.y) / (float)resY) - 1.0f;
	in[2] = 1.0;
	in[3] = -1.0;

	float pos2[4] = { 0 };

	MatrixMul(in, 1, 4, viewProjInv, 4, 4, pos2);
	pos2[3] = 1 / pos2[3];

	pos2[0] *= pos2[3];
	pos2[1] *= pos2[3];
	pos2[2] *= pos2[3];

	float step = 4 * 0.001f;
	float curr = 0.0f;

	float max = step + 1.0f;

	float deltaX = pos1[0] - pos2[0];
	float deltaZ = pos1[1] - pos2[1];
	float deltaY = pos1[2] - pos2[2];



	float w = reinterpret_cast<_TileProjection>(g_gameBase + 0xC4AF0)(_this,
	camera, pos2, pos1, 4);

	int tileX = floor((deltaX * w) + pos2[0]) / 512;
	int tileY = floor((deltaY * w) + pos2[2]) / 512;

	//cout << "w: " << w << endl;
	//cout << "tile: " << dec << tileX << ", " << tileY << endl;


	// reinterpret_cast<_MouseAction>(g_gameBase + 0x0C3240)(_this, "My Walk
	Here", "", -1, 2, -1, 0, tileX, tileY, 0, 0, tileY | (tileX << 16), 0, 1, 0,
	0);


	// g_gameBase + 0x6A6C70
	// int tileX = floor(((deltaX *step) + pos1[0])) / 512;

	// float tileZ = (deltaZ * step) + pos1[1];


	// cout << "camera: " << hex << camera << endl;
	// printf("cameraPos: %.2f, %.2f, %.2f\n", cameraPos[0], cameraPos[1],
	cameraPos[2]);
	// printf("%f, %f, %f\n", cameraPos[0], cameraPos[1], cameraPos[2]);

	//cout << "viewMatrix: " << hex << viewMatrix << endl;
	// cout << "cameraPtr: " << hex << camera << endl;*/
}

// uint64_t _this, uint64_t RDX, uint64_t cursorPos
void __fastcall OnCursorDoAction(fn_hooks::Registers regs) {
	uint64_t action = *reinterpret_cast<uint64_t *>(regs.RDX + 8);
	uint64_t dispatcher = *reinterpret_cast<uint64_t *>(action + 0x40);

	uint64_t dispatcherInstance =
		*reinterpret_cast<uint64_t *>(dispatcher + 0x38);
	if (dispatcherInstance == 0) {
		return;
	}

	uint64_t dispatcherVTable = *reinterpret_cast<uint64_t *>(dispatcherInstance);
	uint64_t dispatcherFunc =
		*reinterpret_cast<uint64_t *>(dispatcherVTable + 0x10);
	uint32_t param1 =
		*reinterpret_cast<uint32_t *>(action + 0x50); // modelId, index
	uint32_t param2 = *reinterpret_cast<uint32_t *>(action + 0x54); // tileX
	uint32_t param3 = *reinterpret_cast<uint32_t *>(action + 0x58); // tileY

	cout << "CursorDoAction: " << hex << dispatcherInstance
		<< ", dispatcher: " << dispatcherFunc << ", Params: " << dec << param1
		<< ", " << param2 << ", " << param3 << endl;

	CursorActionMessage msg;
	msg.set_param1(param1);
	msg.set_param2(param2);
	msg.set_param3(param3);
	server.PushEvent<CursorActionMessage>(msg);
}

void EnableHooks() {
	// mgr.HookFunction<void(int, WPARAM, LPARAM)>(g_gameBase + 0x3789A0,
	// LowLevelMouseProc, 15);
	// mgr.HookFunction<void(int)>(g_gameBase + 0x12B220, OnSendData, 15);

	fn_hooks::InlineHook(signatureByType[DispatcherType::DispatchNetMessage] +
		0x4A,
		OnDispatchNetMessage, 20);
	// mgr.HookFunction<void(uint64_t, uint64_t)>(g_gameBase + 0x3D42F0,
	// glGetUniformLocationHook, 21);

	cout << "CursorWorldContextMenu: " << hex
		<< signatureByType[DispatcherType::CursorWorldContextMenu] << endl;

	if (signatureByType[DispatcherType::CursorWorldContextMenu] == 0) {
		cout << "Could not find CursorWorldContextMenu" << endl;
		return;
	}

	// fn_hooks::InlineHook(signatureByType[DispatcherType::CursorWorldContextMenu], OnCursorWorldContextMenu, 22);

	if (signatureByType[DispatcherType::CursorDoAction] > 0) {
		fn_hooks::InlineHook(signatureByType[DispatcherType::CursorDoAction] + 0x3D,
			OnCursorDoAction, 19); // below 2nd jnz
	}
	else {
		cout << "Could not find CursorDoAction" << endl;
	}
}

std::unordered_map<OptionId, uint64_t> actionContexts;

void FindActions() {
	auto funcs = signaturesByType[DispatcherType::ActionRegister];

	for (auto it = funcs.begin(); it != funcs.end(); ++it) {
		uint64_t funcAddr = *it;
		uint64_t actionCtx = 0;
		uint32_t actionId = 0;

		cs_insn *insn;
		size_t count = cs_disasm(cs_handle, reinterpret_cast<uint8_t *>(funcAddr),
			50, funcAddr, 0, &insn);
		for (int i = 0; i < count; ++i) {
			auto ops = insn[i].detail->x86.operands;
			if (insn[i].id == x86_insn::X86_INS_LEA &&
				ops[0].reg == x86_reg::X86_REG_RAX &&
				ops[1].type == x86_op_type::X86_OP_MEM) {
				actionCtx = insn[i].address + insn[i].size + ops[1].mem.disp;
			}
			else if (insn[i].id == x86_insn::X86_INS_MOV &&
				ops[0].mem.base == x86_reg::X86_REG_RIP) {
				if (actionId == 0) {
					actionId = ops[1].imm;
					break;
				}
			}
		}

		if (actionCtx != 0 && actionId != 0) {
			actionContexts[static_cast<OptionId>(actionId)] = actionCtx;
		}

		cs_free(insn, count);
	}
}

std::unordered_map<MessageType, uint64_t> msgDefs;

void FindMessages() {
	auto funcs = signaturesByType[DispatcherType::MessageDefRegister];

	for (auto it = funcs.begin(); it != funcs.end(); ++it) {
		uint64_t funcAddr = *it;
		uint64_t def = 0;

		cs_insn *insn;
		size_t count = cs_disasm(cs_handle, reinterpret_cast<uint8_t *>(funcAddr),
			50, funcAddr, 0, &insn);
		for (int i = 0; i < count; ++i) {
			auto ops = insn[i].detail->x86.operands;
			if (insn[i].id == x86_insn::X86_INS_LEA &&
				ops[0].reg == x86_reg::X86_REG_RAX &&
				ops[1].type == x86_op_type::X86_OP_MEM) {
				def = insn[i].address + insn[i].size + ops[1].mem.disp;
			}
		}

		if (def != 0) {
			uint32_t id = *reinterpret_cast<uint32_t *>(def);
			cout << "id: " << dec << id << ", def " << hex << def << endl;
			msgDefs[static_cast<MessageType>(id)] = def;
		}

		cs_free(insn, count);
	}
}

int yr_scan_callback(int message, void *message_data, void *user_data) {
	if (message == CALLBACK_MSG_RULE_MATCHING) {
		YR_RULE *rule = (YR_RULE *)message_data;

		int64_t index = 0;
		int64_t id = 0;

		bool multiple = false;

		YR_META *meta;

		yr_rule_metas_foreach(rule, meta) {
			if (strcmp(meta->identifier, "id") == 0) {
				id = meta->integer;
			}
			else if (strcmp(meta->identifier, "index") == 0) {
				index = meta->integer;
			}
			else if (strcmp(meta->identifier, "multiple") == 0) {
				multiple = static_cast<bool>(meta->integer);
			}
		}

		YR_STRING *string = &rule->strings[0];

		uint64_t matchLocation = 0;

		YR_MATCH *match;
		int i = 0;
		yr_string_matches_foreach(string, match) {
			matchLocation = g_gameBase + match->base + match->offset;
			printf("Found rule %s at 0x%llx\n", rule->identifier, matchLocation);
			if (multiple) {
				signaturesByType[static_cast<DispatcherType>(id)].push_back(
					matchLocation);
			}
			else if (index == i) {
				signatureByType[static_cast<DispatcherType>(id)] = matchLocation;
				signs[matchLocation] = id;
				break;
			}
			i++;
		}

		/*printf("Found rule %s at 0x%llx\n", rule->identifier, matchLocation);
		if (matchLocation != 0 && multiple) {
				signs[matchLocation] = id;
				signatureByType[static_cast<DispatcherType>(id)] = matchLocation;
		}*/
	}
	else if (message == CALLBACK_MSG_SCAN_FINISHED) {
		EnableHooks();
		// FindActions();
		// FindMessages();
	}

	return CALLBACK_CONTINUE;
}

HWND g_gameHwnd;

DWORD WINAPI MainThread(LPVOID lParam) {
	CreateConsole();
	if (!InitializeCapstone()) {
		cout << "Could not initialize capstone\n";
	}

	if (!InitializeYara()) {
		cout << "Could not initialize yara\n";
	}

	g_gameBase = reinterpret_cast<DWORD_PTR>(GetModuleHandle("rs2client.exe"));
	g_opengl = reinterpret_cast<DWORD_PTR>(GetModuleHandle("opengl32.dll"));

	// _wglGetProcAddress =
	// reinterpret_cast<__wglGetProcAddress>(GetProcAddress((HMODULE)g_opengl,
	// "wglGetProcAddress"));
	// void *ul =
	// reinterpret_cast<__wglGetProcAddress>(f)("glGetUniformLocation");
	//_wglGetProcAddress("glGetUniformLocation");
	// cout << "getUniformLocation: " << hex << (int)ul << endl;

	MODULEINFO modInfo = { 0 };
	GetModuleInformation(GetCurrentProcess(), (HMODULE)g_gameBase, &modInfo,
		sizeof(modInfo));

	yr_rules_scan_mem(yrRules, (uint8_t *)g_gameBase, modInfo.SizeOfImage, NULL,
		yr_scan_callback, NULL, NULL);

	UnitTests_Unpack();

	HWND parHwnd = FindWindow("JagWindow", NULL);
	g_gameHwnd = FindWindowEx(parHwnd, NULL, "JagOpenGLView", NULL);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH: {
		DWORD thdId = 0;
		CreateThread(NULL, 0, MainThread, nullptr, 0, &thdId);
	} break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
