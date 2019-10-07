#pragma once
#include <stdint.h>
#include <iostream>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <string>
#include <set>
#include <queue>

#include "containers.h"
#include "invalid_pointer_exception.h"

#define ENTITY_TYPE_OFF 0x40
#define ENTITY_ID_OFF 0x20


enum class DispatcherType {
	GameMessage = 1,
	SetInventorySlot = 2,
	UpdateObject = 3,
	SetSkillXP = 4,
	SetAccountSecuritySettings = 5,
	SetGameState = 6,
	SetGameData = 7,
	SetLobbyNews = 8,
	SetWorlds = 9,
	UpdateNPC = 10,
	LoadWorld = 11,
	CursorWorldContextMenu = 12,
	UpdatePlayers = 13,
	CursorDoAction = 14,
	InteractNPC = 15,
	DispatchNetMessage = 16,
	InteractObject = 17,
	ActionRegister = 18,
	MessageDefRegister = 19,
	SetPlayerHP = 20,
	OpenDialog = 21
};

enum class OptionId {
	ObjectOption1 = 3,
	ObjectOption2 = 4,
	ObjectOption3 = 5,
	ObjectOption4 = 6,
	NPCOption1 = 9,
	NPCAttack = 10,
	NPCOption2 = 11,
	NPCOption3 =  12,
	NPCOption4 = 13,
	WorldItemOption1 = 20,
	WorldItemOption2 = 21,
	WorldItemOption3 = 22,
	MoveToTile = 23,
	UIAction = 57,
	ObjectExamine = 1002,
	NPCExamine = 1003,
	DropItem = 1007
};

enum class MessageType {
	FishingSpotOption1 = 19, // 0x12 0x03 0x78
	FishingSpotExamine = 39,
	Object2Option1 = 10,
	ObjectOption1 = 0x20,
	ObjectOption2 = 102,
	ObjectOption3 = 21,
	ObjectOption4 = 120,
	NPCOption1 = 32,
	NPCAttack = 92,
	NPCOption2 = 34,
	NPCOption3 = 51,
	NPCOption4 = 121,
	WorldItemOption1 = 20,
	WorldItemOption2 = 81,
	ObjectExamine = 40,
	NPCExamine = 33
};

enum class NPCAttributes {
	CurrentHP = 3,
	MaxHP = 9,
	Level = 12
};


extern uint64_t g_gameBase;
extern uint64_t g_entityList;
extern uint64_t g_entityListFull;
extern uint64_t* g_contextStore;
extern uint64_t g_gameContext;
extern HWND g_gameHwnd;


extern std::unordered_map<DispatcherType, uint64_t> signatureByType;
extern std::unordered_map<OptionId, uint64_t> actionContexts;
extern std::unordered_map<MessageType, uint64_t> msgDefs;


enum InventoryOffset {
	BaseOffset = 0x6A1018 + 8, // rax of InventoryConstructor
	List = 0x1158, // Doesn't change usually
};

#pragma pack(push)
struct dataStruct {
	uint64_t unk1;
	uint8_t *dataPtr;
};
#pragma pack(pop)

inline const uint64_t hash_64(const uint32_t in) {
	uint64_t p1 = (in & 0xFF) ^ 0xCBF29CE484222325;
	uint64_t p2 = (in & 0xFF00) ^ 0x100000001B3;
	uint64_t p3 = (in & 0xFF0000) ^ 0x100000001B3;
	uint64_t p4 = (in & 0xFF000000) ^ 0x100000001B3;
	return p1 * p2 * p3 * p4;
}

inline void DoAntiAFK() {
	// Cheap way to prevent afk causing disconnect,
	// by calling this function when performing action

	SendMessage(g_gameHwnd, WM_KEYDOWN, VK_INSERT, 0);
	SendMessage(g_gameHwnd, WM_KEYUP, VK_INSERT, 0);
}


/*inline uint64_t ReadPointer(uint64_t address, uint64_t offset = 0) {
	const bool is_aligned = (address % sizeof(uint64_t)) == 0;
	if (address == 0 || !is_aligned) {
		throw InvalidPointerException("in " __FILE__ " at line " + std::to_string(__LINE__));
	}

	return *reinterpret_cast<uint64_t*>(address + offset);
}

template <typename T>
inline uint64_t ReadPointer(T* ptr, uint64_t offset) {
	return ReadPointer(reinterpret_cast<uint64_t*>(ptr), offset);
}*/


#define ReadPtrOffset(address, x) (address == 0 || (address % sizeof(uint64_t)) != 0) ? throw InvalidPointerException(std::to_string(address) + " in " __FUNCTION__ + ", " + __FILE__" at line " + std::to_string(__LINE__)) : *(uint64_t*)(address + x)
#define ReadPtr(address) ReadPtrOffset(address, 0)

namespace Game {

	inline void GetServerEntities(std::vector<uint64_t> *out) {
		uint64_t list = ReadPtrOffset(ReadPtrOffset(g_gameContext, 8), 0x10D8);


		uint64_t arr = ReadPtrOffset(list, 0x20);
		std::cout << "arr: " << std::hex << arr << std::endl;

		uint32_t count = *reinterpret_cast<uint32_t*>(list + 0x28);
		std::cout << "count: " << std::dec << count << std::endl;

		// cmp     edi, [rbx+0B0B0h]
		uint32_t id_count = *reinterpret_cast<uint32_t*>(list + 0xB0B0);

		// std::cout << "id_count: " << std::dec << id_count << std::endl;

		for (uint32_t i = 0; i < id_count; ++i) {

			// movsxd  r8, dword ptr [rbx+rdi*4+0A0B0h]
			uint32_t id = *reinterpret_cast<uint32_t*>(list + 4 * i + 0xA0B0);
			uint32_t index = id % count;


			uint64_t entry_ptr = *reinterpret_cast<uint64_t*>(arr + index * 8);
			if (entry_ptr != 0) {
				while (id != *reinterpret_cast<uint32_t*>(entry_ptr)) {
					entry_ptr = *reinterpret_cast<uint64_t*>(entry_ptr + 0x18);
					if (entry_ptr == 0) {
						break;
					}
				}

				// std::cout << "entry: " << std::hex << entry_ptr << ", id: " << std::dec << type << std::endl;

				uint64_t entity_ptr = ReadPtrOffset(ReadPtrOffset(entry_ptr, 8), 24);
				uint32_t type = *reinterpret_cast<uint32_t*>(entity_ptr + 0x40);
				out->push_back(entity_ptr);
			}
		}
	}

	inline uint64_t GetControlledPlayer() {
		uint64_t ptr1 = ReadPtrOffset(ReadPtrOffset(g_gameContext, 8), 0x16C0);
		uint64_t ptr2 = ReadPtrOffset(ptr1, 0x58);

		int32_t idx = *(int*)(ptr1 + 0x48);
		if (ReadPtrOffset(ptr1, 0x58) == 0) {
			if (idx == -1) {
				return 0;
			}

			uint64_t ptr2 = ReadPtrOffset(
				ReadPtrOffset(ReadPtrOffset(ReadPtr(ptr1), 0x10F8), 0x10), 8 * idx
			);
			return ptr2 != 0 ? ReadPtrOffset(ptr2, 0x38) : 0;
		}

		uint64_t ptr3 = ReadPtrOffset(ptr1, 0x50);
		return ReadPtrOffset(ptr3, 8);
	}

	inline void GetUnkEntities(std::vector<uint64_t> *out) {
		uint64_t list = ReadPtrOffset(ReadPtrOffset(g_gameContext, 8), 0x10F8);


		uint64_t start_ptr = ReadPtrOffset(list, 0x4030);
		uint64_t end_ptr = ReadPtrOffset(list, 0x4038);

		size_t count = (end_ptr - start_ptr) / 8;

		std::cout << "arr_beg: " << std::hex << start_ptr << std::endl;

		std::cout << "count: " << std::dec << count << std::endl;

		for (int i = 0; i < count; ++i) {
			uint64_t entry_ptr = ReadPtrOffset(start_ptr, i * 8);
			if (entry_ptr == 0) {
				continue;
			}

			out->push_back(entry_ptr);
			std::cout << "entry: " << std::hex << entry_ptr << std::endl;
		}
	}

	inline void GetStaticEntities_(uint64_t curr, std::set<uint64_t> *out) {
		uint64_t list = curr;

		// std::cout << "List ptr: " << std::hex << list << std::endl;

		uint64_t entity_ptr = ReadPtrOffset(list, 0x198);
		if (entity_ptr != 0) {
			// std::cout << "entity_ptr: " << std::hex << entity_ptr << std::endl;
			out->insert(entity_ptr);
		}
		if (**reinterpret_cast<uint8_t**>(list + 0xF8) & 0x20) {
			uint64_t start_ptr = ReadPtrOffset(list, 0x130);
			uint64_t end_ptr = ReadPtrOffset(list, 0x138);

			if (start_ptr != end_ptr) {
				size_t count = (end_ptr - start_ptr + 7) >> 3;

				// std::cout << "arr_start: " << std::hex << start_ptr << std::endl;

				// std::cout << "count: " << std::dec << count << std::endl;


				for (int i = 0; i < count; ++i) {
					GetStaticEntities_(ReadPtrOffset(start_ptr, i * 8), out);
				}
			}
		}

	}

#pragma pack(push)
	struct ListEntry {
		uint64_t unk1;
		uint64_t item;
	};
#pragma pack(pop)


	inline void GetVisibleEntities(std::vector<uint64_t> *out) {
		ListEntry *start_ptr = reinterpret_cast<ListEntry*>(ReadPtr(g_entityList));
		ListEntry *end_ptr = reinterpret_cast<ListEntry*>(ReadPtrOffset(g_entityList, 8));

		const std::vector<ListEntry> entries(start_ptr, end_ptr);
		for (const ListEntry &list_entry : entries) {
			out->push_back(list_entry.item);
		}
	}

	inline void GetStaticEntities(std::set<uint64_t> *out) {
		// std::cout << "g_entityListFull: " << std::hex << g_entityListFull << std::endl;
		GetStaticEntities_(g_entityListFull, out);
	}

	inline bool GetProperty(uint16_t id, uint32_t *out) {
		// RCX SetResource in IDA
		uint64_t list = ReadPtrOffset(g_gameContext, 8) + 0x16D0 + 0x20;

		uint64_t list_start = ReadPtrOffset(list, 8);
		uint64_t count = ReadPtrOffset(list, 16);
		uint64_t list_end = list_start + count * 8;

		uint16_t index = id % count;

		uint64_t curr = ReadPtrOffset(list_start, 8 * index);

		while (curr != 0 && *reinterpret_cast<uint32_t*>(curr) != id) {
			curr = ReadPtrOffset(curr, 0x50);
		}
		if (curr == 0) {
			return false;
		}

		/*cout << "list: " << hex << list << endl;
		cout << "list start: " << hex << list_start << endl;
		cout << "list end: " << hex << list_end << endl;
		cout << "count: " << dec << count << endl;
		cout << "curr: " << hex << curr << endl;*/

		*out = *reinterpret_cast<uint32_t*>(curr + 0x30);
		return true;
	}

	inline bool GetUIWidget(const uint32_t id, uint64_t *out) {
		// FindUI in IDA
		const uint64_t list = ReadPtrOffset(ReadPtrOffset(g_gameContext, 8), 0x1098) + 0x128;

		const uint64_t list_start = ReadPtrOffset(list, 8);
		const uint64_t count = ReadPtrOffset(list, 16);
		const uint64_t list_end = list_start + count * 8;


		const uint32_t index = id % count;

		uint64_t curr = ReadPtrOffset(list_start, 8 * index);

		/*std::cout << "list: " << std::hex << list << std::endl;
		std::cout << "list start: " << std::hex << list_start << std::endl;
		std::cout << "list end: " << std::hex << list_end << std::endl;
		std::cout << "count: " << std::dec << count << std::endl;*/

		while (curr != 0 && *reinterpret_cast<uint32_t*>(curr) != id) {
			// std::cout << "cid: " << std::hex << *reinterpret_cast<uint32_t*>(curr) << std::endl;
			curr = ReadPtrOffset(curr, 0x18);
		}
		if (curr == 0) {
			return false;
		}


		// std::cout << "curr: " << std::hex << curr << std::endl;

		*out = ReadPtrOffset(curr, 0x10);
		return true;
	}

	inline bool GetUIWidgetParent(const uint16_t parent_id, uint64_t *out) {
		// GetUIWidgetParentById in IDA
		// lea  rcx, [r14+18h]
		// call GetUIWidgetParentById
		// add  rcx, 58h
		// call FindUI
		const uint64_t list = ReadPtrOffset(ReadPtrOffset(g_gameContext, 8), 0x1098) + 0x18 + 0x58;

		const uint64_t list_start = ReadPtrOffset(list, 8);
		const uint64_t count = ReadPtrOffset(list, 16);
		const uint64_t list_end = list_start + count * 8;


		const uint32_t index = parent_id % count;

		uint64_t curr = ReadPtrOffset(list_start, 8 * index);

		while (curr != 0 && *reinterpret_cast<uint32_t*>(curr) != parent_id) {
			curr = ReadPtrOffset(curr, 0x18);
		}
		if (curr == 0) {
			return false;
		}

		*out = ReadPtrOffset(curr, 0x10);
		return true;
	}

	class UIWidgetDef {

		enum class Offsets {
			name = 0x98,
			options = 0xD8
		};

	public:
		UIWidgetDef(const uint64_t base = 0) : base_(base) {}

		struct Option {
			int index;
			std::string text;
		};


		const std::string name() const {
			return std::string(*reinterpret_cast<const char**>(base_ + static_cast<int>(Offsets::name)));
		}

		const uint64_t base() const {
			return base_;
		}

		void SetBase(const uint64_t base) {
			base_ = base;
		}


		const std::vector<Option> options() const {
			const size_t opt_size = 64;
			const uint64_t opt_start = ReadPtrOffset(base_, static_cast<int>(Offsets::options));
			const uint64_t opt_end = ReadPtrOffset(base_, static_cast<int>(Offsets::options) + 8);

			size_t opt_count = (opt_end - opt_start) / opt_size;

			std::vector<Option> options;

			// std::cout << "opt_start: " << std::hex << opt_start << std::endl;
			// std::cout << "opt_count: " << opt_count << std::endl;

			// Item specific options
			for (int i = 0; i < opt_count; ++i) {
				uint64_t opt = opt_start + i * opt_size;

				const uint64_t text_start = *reinterpret_cast<uint64_t*>(opt);
				const uint64_t text_end = *reinterpret_cast<uint64_t*>(opt + 8);

				size_t text_length = text_end - text_start;
				if (text_length > 0) {
					// std::cout << "option " << std::dec << i << ": " << (const char*)(text_start) << std::endl;
					options.push_back({ i + 1, reinterpret_cast<const char*>(text_start) });
				}
			}
			return options;
		}

	private:
		uint64_t base_;
	};

	inline bool GetUIWidgetChild(const uint32_t id, UIWidgetDef *out) {

		uint64_t parent_ptr;
		const uint16_t parent_id = id >> 16;
		const uint16_t child_id = id & 0xFFFF;

		if (!GetUIWidgetParent(parent_id, &parent_ptr)) {
			return false;
		}

		const uint64_t child_ptr_array = ReadPtrOffset(parent_ptr, 8);
		// std::cout << "child_arr: " << std::hex << child_ptr_array << std::endl;
		// std::cout << "child_id: " << std::hex << child_id << std::endl;
		std::cout << "child_ptr: " << std::hex << (child_ptr_array + child_id * 24 + 16) << std::endl;
		out->SetBase(ReadPtrOffset(child_ptr_array, child_id * 24 + 16));
		return true;
	}


	class ObjectDef {

		enum class Offsets {
			id = 0x20,
			name = 0x1E0,
			name2 = 0x1E8,
			options = 0x38
		};

	public:
		ObjectDef(const uint64_t base = 0) : base_(base) {}

		struct Option {
			int index;
			std::string text;
		};

		const uint32_t id() const {
			return *reinterpret_cast<uint32_t*>(base_ + static_cast<int>(Offsets::id));
		}

		const std::string name() const {
			return std::string(*reinterpret_cast<const char**>(base_ + static_cast<int>(Offsets::name)));
		}

		const std::string name2() const {
			return std::string(*reinterpret_cast<const char**>(base_ + static_cast<int>(Offsets::name2)));
		}

		const std::vector<Option> options() const {
			uint64_t opt_start = ReadPtrOffset(base_, static_cast<int>(Offsets::options));
			uint64_t opt_end = ReadPtrOffset(base_, static_cast<int>(Offsets::options) + 8);
			size_t opt_size = 64;
			size_t opt_count = (opt_end - opt_start) / opt_size;

			std::vector<Option> options;

			for (int i = 0; i < opt_count; ++i) {
				uint64_t opt = opt_start + i * opt_size;
				uint64_t text_start = *reinterpret_cast<uint64_t*>(opt);
				uint64_t text_end = *reinterpret_cast<uint64_t*>(opt + 8);
				size_t text_length = text_end - text_start;
				if (text_length > 0) {
					const char *text = (const char*)text_start;
					// cout << "text_ptr object2: " << text << ", " << "length: " << dec << text_length << ", " << hex << text_start << endl;
					options.push_back({ i, text });
				}
			}
			return options;
		}

		void SetBase(const uint64_t base) {
			base_ = base;
		}

	private:
		uint64_t base_;
	};

	class NPCDef {

		enum class Offsets {
			id = 0x20,
			name = 0x1E0,
			options = 0x38
		};



	public:
		NPCDef(const uint64_t base = 0) : base_(base) {}

		struct Option {
			int index;
			std::string text;
		};

		const uint32_t id() const {
			return *reinterpret_cast<uint32_t*>(base_ + static_cast<int>(Offsets::id));
		}

		const std::string name() const {
			return std::string(*reinterpret_cast<const char**>(base_ + static_cast<int>(Offsets::name)));
		}

		const std::vector<Option> options() const {
			uint64_t opt_start = base_ + static_cast<int>(Offsets::options);
			
			size_t opt_size = 64;
			size_t opt_count = 6;

			uint64_t opt_end = opt_start + opt_count * opt_size;

			std::vector<Option> options;

			for (int i = 0; i < opt_count; ++i) {
				uint64_t opt = opt_start + i * opt_size;
				uint64_t text_start = *reinterpret_cast<uint64_t*>(opt);
				uint64_t text_end = *reinterpret_cast<uint64_t*>(opt + 8);
				size_t text_length = text_end - text_start;
				if (text_length > 0) {
					const char *text = (const char*)text_start;
					options.push_back({ i, text });
				}
			}
			return options;
		}

		void SetBase(const uint64_t base) {
			base_ = base;
		}

	private:
		uint64_t base_;
	};


	inline bool GetObjectDef(const uint32_t id, ObjectDef *out) {
		const uint64_t entityStart = *reinterpret_cast<uint64_t*>(g_entityList);
		const uint64_t entityEnd = *reinterpret_cast<uint64_t*>(g_entityList + 8);
		const size_t entityCount = (entityEnd - entityStart) / 16;

		for (int i = 0; i < entityCount; ++i) {
			const uint64_t entityPtr = *reinterpret_cast<uint64_t*>(entityStart + i * 16 + 8);
			if (entityPtr == 0) {
				continue;
			}
			
			const uint32_t type = *reinterpret_cast<uint32_t*>(entityPtr + ENTITY_TYPE_OFF);
			if (type == 12) {
				const uint64_t objDef = *reinterpret_cast<uint64_t*>(entityPtr + 0xF8 + 8);
				if (objDef == 0) {
					continue;
				}
				if (*reinterpret_cast<uint32_t*>(objDef + ENTITY_ID_OFF) == id) {
					out->SetBase(objDef);
					return true;
				}
			} else if (type == 0) {
				const uint64_t objDef = *reinterpret_cast<uint64_t*>(entityPtr + 0x110 + 8);
				if (objDef == 0) {
					continue;
				}

				if (*reinterpret_cast<uint32_t*>(objDef + ENTITY_ID_OFF) == id) {
					out->SetBase(objDef);
					return true;
				}
			}
		}
		return false;
	}

	inline bool GetNPCDef(const uint32_t id, NPCDef *out) {
		std::vector<uint64_t> entities;
		GetServerEntities(&entities);

		for (const uint64_t &entity_ptr : entities) {
			uint32_t type = *reinterpret_cast<uint32_t*>(entity_ptr + ENTITY_TYPE_OFF);
			if (type == 1) {
				if (*reinterpret_cast<uint32_t*>(entity_ptr + 0xF8) == id) {
					const uint64_t npc_def = *reinterpret_cast<uint64_t*>(entity_ptr + 0x1010);
					if (npc_def == 0) {
						continue;
					}

					out->SetBase(npc_def);
					return true;
				}
			}
		}
		return false;
	}



	inline bool InteractObject(const uint32_t id,
		const uint32_t tile_x, const uint32_t tile_y,
		const std::string &option_text) {

		std::cout << "Interact with " << std::dec << id << " located at " << tile_x << ", " << tile_y << std::endl;

		uint8_t data[100] = { 0 };

		*reinterpret_cast<uint32_t*>(&data[0x50]) = id;
		*reinterpret_cast<uint32_t*>(&data[0x54]) = tile_x;
		*reinterpret_cast<uint32_t*>(&data[0x58]) = tile_y;

		dataStruct dt;
		dt.dataPtr = data;

		DoAntiAFK();

		ObjectDef object_def;
		if (!GetObjectDef(id, &object_def)) {
			std::cout << "Could not find object" << std::endl;
			return false;
		}

		const std::vector<ObjectDef::Option> options = object_def.options();

		std::vector<ObjectDef::Option> filtered;
		std::copy_if(options.begin(), options.end(), std::back_inserter(filtered), [option_text](const ObjectDef::Option &opt) {
			return opt.text.compare(option_text) == 0;
		});

		if (filtered.size() == 0) {
			return false;
		}

		const ObjectDef::Option &option = filtered[0];

		const uint64_t option_handler = ReadPtrOffset(ReadPtrOffset(
			ReadPtrOffset(g_gameContext, 0xFB8), 8 * option.index), 0x38);

		const uint64_t option_handler_vtable = *reinterpret_cast<uint64_t*>(option_handler);
		const uint64_t do_option_func = *reinterpret_cast<uint64_t*>(option_handler_vtable + 0x10);

		typedef float(__cdecl *_DoOption)(uint64_t _this, void *dataPtr);
		reinterpret_cast<_DoOption>(do_option_func)(option_handler, &dt);
		return true;
	}

	enum class InventoryType : uint16_t {
		Backpack = 93,
		Bank = 95,
		CoinPouch = 623
	};

#pragma pack(push)
	struct InventoryItem {
		int32_t id;
		uint32_t quantity;
	};
#pragma pack(pop)

	inline uint64_t GetInventory() {
		// uint64_t ptr = *reinterpret_cast<uint64_t*>(g_gameBase + InventoryOffset::BaseOffset);
		return ReadPtrOffset(g_gameBase, InventoryOffset::BaseOffset);
	}

	inline uint64_t GetInventoryList() {
		return *reinterpret_cast<uint64_t*>(*reinterpret_cast<uint64_t*>(g_gameContext + 8) + InventoryOffset::List);
	}

	inline uint64_t GetInventoryByType(const uint32_t type) {
		const uint32_t _type = type * 2;

		uint64_t list_ptr = GetInventoryList() + 8;
		//std::cout << "inventoryList: " << std::hex << inventoryList << std::endl;

		uint64_t array_ptr = ReadPtrOffset(list_ptr, 0x18);
		uint64_t tail = ReadPtrOffset(list_ptr, 8);

		uint64_t type_hash = hash_64(_type);
		uint64_t count = ReadPtrOffset(list_ptr, 0x30);

		uint64_t index = 2 * (type_hash & count);

		if (array_ptr == 0) {
			throw InvalidPointerException("arrayPtr in GetInventoryByType is null. Check inventory pointer.");
		}

		uint64_t first_ptr = ReadPtrOffset(array_ptr, 8 * index);
		uint64_t curr_ptr = first_ptr;


		uint32_t curr_type = 0;

		while (1) {
			curr_type = *reinterpret_cast<uint32_t*>(curr_ptr + 0x10);
			if (curr_type == _type) {
				break;
			}
			uint64_t next_ptr = *reinterpret_cast<uint64_t*>(curr_ptr);
			if (next_ptr == first_ptr) {
				break;
			}
			curr_ptr = next_ptr;
		}

		return curr_type == _type ? ReadPtrOffset(curr_ptr, 0x20) : 0;
	}

	inline const JagexList<InventoryItem> GetBackpack() {
		return JagexList<InventoryItem>(GetInventoryByType(static_cast<uint32_t>(Game::InventoryType::Backpack)));
	}

	inline const JagexList<InventoryItem> GetInventory(const uint32_t type) {
		return JagexList<InventoryItem>(GetInventoryByType(type));
	}


	class ItemDef {

		enum class Offsets {
			id = 0x20,
			name = 0x198,
			options = 0xD8
		};

	public:
		ItemDef(const uint64_t base) : base_(base) {}

		struct Option {
			int index;
			std::string text;
		};


		const uint32_t id() const {
			return *reinterpret_cast<uint32_t*>(base_ + static_cast<int>(Offsets::id));
		}

		const std::string name() const {
			return std::string(*reinterpret_cast<const char**>(base_ + static_cast<int>(Offsets::name)));
		}

		const uint64_t base() const {
			return base_;
		}

		const std::vector<Option> ground_options() const {
			uint64_t opt_start = base_ + static_cast<int>(Offsets::options) - 5 * 32;

			size_t opt_size = 32;
			size_t opt_count = 6;


			std::vector<Option> options;

			for (int i = 0; i < opt_count; ++i) {
				uint64_t opt = opt_start + i * opt_size;

				uint64_t text_start = *reinterpret_cast<uint64_t*>(opt);
				uint64_t text_end = *reinterpret_cast<uint64_t*>(opt + 8);
				size_t text_length = text_end - text_start;
				if (text_length > 0) {

					const char *text = (const char*)text_start;
					std::cout << "text: " << text << std::endl;
					options.push_back({ i, text });
				}
			}

			return options;
		}

		const std::vector<Option> backpack_options() const {
			size_t opt_size = 32;
			uint64_t opt_start = base_ + static_cast<int>(Offsets::options);

			size_t opt_count = 6;

			std::vector<Option> options;

			// std::cout << "opt_start: " << std::hex << opt_start << std::endl;

			// Item specific options
			for (int i = 1; i < opt_count; ++i) {
				uint64_t opt = opt_start + i * opt_size;

				uint64_t text_start = *reinterpret_cast<uint64_t*>(opt);
				uint64_t text_end = *reinterpret_cast<uint64_t*>(opt + 8);
				size_t text_length = text_end - text_start;
				if (text_length > 0) {
					// std::cout << "option " << std::dec << i << ", " << y << ": " << (const char*)(text_start) << std::endl;
					options.push_back({i, reinterpret_cast<const char*>(text_start) });
				}
			}

			// Generic options
			options.push_back({ 0, "Use" });
			options.push_back({ 7, "Examine" });

			return options;
		}

	private:
		const uint64_t base_;
	};


	inline const ItemDef GetItemDef(const uint32_t id) {
		uint64_t ctx = g_contextStore[0x0A];
		uint64_t _this = ReadPtrOffset(ctx, 0x38);
		uint64_t vtable = ReadPtr(_this);
		uint64_t fn_ptr = ReadPtrOffset(vtable, 0x38);

		typedef uint64_t(__cdecl *_GetItemDef)(uint64_t _this, uint32_t id, int unk);
		return ItemDef(*reinterpret_cast<uint64_t*>(reinterpret_cast<_GetItemDef>(fn_ptr)(_this, id, 0) + 8));
	}

	inline void GetItemDefsOnTile(const uint64_t tile, std::vector<ItemDef> *items) {
		uint64_t arr_ptr = *reinterpret_cast<uint64_t*>(tile + 0xE8);
		uint64_t arr_end_ptr = *reinterpret_cast<uint64_t*>(tile + 0xF0);

		uint32_t count = (arr_end_ptr - arr_ptr) / 16;

		for (uint32_t i = 0; i < count; ++i) {
			uint32_t id = **reinterpret_cast<uint32_t**>(arr_ptr + (i * 16) + 8);
			items->push_back(GetItemDef(id));
		}
	}
	

	inline void DoUIAction(const uint32_t option_id, const int32_t param2, const uint32_t id) {
		uint8_t data[100] = { 0 };

		*reinterpret_cast<uint32_t*>(&data[0x50]) = option_id;
		*reinterpret_cast<uint32_t*>(&data[0x54]) = param2;
		*reinterpret_cast<uint32_t*>(&data[0x58]) = id;


		// UIMouseAction
		uint64_t option_handler = *reinterpret_cast<uint64_t*>(g_gameBase + 0x6C9E80 + 0x38);
		uint64_t option_handler_vtable = *reinterpret_cast<uint64_t*>(option_handler);
		uint64_t handler_func = *reinterpret_cast<uint64_t*>(option_handler_vtable + 0x10);

		std::cout << "DoUIAction: " << std::dec << option_id
			<< ", " << param2 << ", " << id << ", " << std::hex
			<< option_handler << ", " << std::hex << handler_func << std::endl;

		dataStruct dt;
		dt.dataPtr = data;

		typedef float(__cdecl *_DoUIAction)(uint64_t _this, void *dataPtr);
		reinterpret_cast<_DoUIAction>(handler_func)(option_handler, &dt);
	}

	inline bool DoUIAction(const std::string &option_text, const int32_t param2, const uint32_t id) {

		Game::UIWidgetDef widget;
		if (!Game::GetUIWidgetChild(id, &widget)) {
			return false;
		}

		const std::vector<UIWidgetDef::Option> options = widget.options();

		std::vector<UIWidgetDef::Option> filtered;
		std::copy_if(options.begin(), options.end(), std::back_inserter(filtered), [option_text](const UIWidgetDef::Option &option) {
			return option.text.compare(option_text) == 0;
		});

		if (filtered.size() == 0) {
			return false;
		}

		DoUIAction(filtered[0].index, param2, id);
		return true;
	}

	inline bool BackpackInteractItem(uint32_t slot, const std::string &option_text) {
		if (slot >= 28) {
			return false;
		}

		uint8_t data[100] = { 0 };

		const InventoryItem &item = Game::GetBackpack()[slot];

		const ItemDef item_def = GetItemDef(item.id);
		const std::vector<ItemDef::Option> options = item_def.backpack_options();

		std::vector<ItemDef::Option> filtered;
		std::copy_if(options.begin(), options.end(), std::back_inserter(filtered), [option_text](const ItemDef::Option &option) {
			return option.text.compare(option_text) == 0;
		});

		if (filtered.size() == 0) {
			return false;
		}

		const ItemDef::Option &option = filtered[0];

		uint32_t option_id = 0;

		// std::cout << "Option index: " << option.index << std::endl;

		option_id = option.index > 3 ? option.index + 3 : option.index;

		DoUIAction(option_id, slot, 0x5C10005);
		return true;
	}

	inline bool InteractGroundItem(uint32_t id, int32_t tile_x, int32_t tile_y, const std::string &option_text) {
		uint8_t data[100] = { 0 };

		*reinterpret_cast<uint32_t*>(&data[0x50]) = id;
		*reinterpret_cast<uint32_t*>(&data[0x54]) = tile_x;
		*reinterpret_cast<uint32_t*>(&data[0x58]) = tile_y;

		const ItemDef item_def = GetItemDef(id);
		const std::vector<ItemDef::Option> options = item_def.ground_options();

		std::vector<ItemDef::Option> filtered;
		std::copy_if(options.begin(), options.end(), std::back_inserter(filtered), [option_text](const ItemDef::Option &option) {
			return option.text.compare(option_text) == 0;
		});

		if (filtered.size() == 0) {
			return false;
		}

		const ItemDef::Option &option = filtered[0];
		uint64_t option_handler = *reinterpret_cast<uint64_t*>(*reinterpret_cast<uint64_t*>(*reinterpret_cast<uint64_t*>(g_gameContext + 0xF58) + 8 * option.index) + 0x38);
		uint64_t option_handler_vtable = *reinterpret_cast<uint64_t*>(option_handler);
		uint64_t do_option_func = *reinterpret_cast<uint64_t*>(option_handler_vtable + 0x10);

		dataStruct dt;
		dt.dataPtr = data;

		typedef float(__cdecl *_DoOption)(uint64_t _this, void *dataPtr);
		reinterpret_cast<_DoOption>(do_option_func)(option_handler, &dt);
		return true;
	}

	
	inline void DoItemInterfaceAction(int32_t param1, int32_t param2, uint32_t id) {
		uint8_t data[100] = { 0 };

		*reinterpret_cast<uint32_t*>(&data[0x50]) = param1;
		*reinterpret_cast<uint32_t*>(&data[0x54]) = param2;
		*reinterpret_cast<uint32_t*>(&data[0x58]) = id;

		// UIMouseAction last case
		uint64_t option_handler = *reinterpret_cast<uint64_t*>(g_gameBase + 0x6C9D40 + 0x38);
		uint64_t option_handler_vtable = *reinterpret_cast<uint64_t*>(option_handler);
		uint64_t handler_func = *reinterpret_cast<uint64_t*>(option_handler_vtable + 0x10);

		std::cout << "DoItemInterfaceAction: " << std::dec << param1 << ", " << std::dec << param2 << ", " << id << ", " << std::hex << option_handler << ", " << std::hex << handler_func << std::endl;

		dataStruct dt;
		dt.dataPtr = data;

		typedef float(__cdecl *_DoUIAction)(uint64_t _this, void *dataPtr);
		reinterpret_cast<_DoUIAction>(handler_func)(option_handler, &dt);
	}

	inline void WorldToTilePos(const int32_t wX, const int32_t wY, int32_t *tX, int32_t *tY) {
		*tX = (wX / 512) & 0xFFF;
		*tY = (wY / 512) & 0xFFF;
	}

	

	inline bool InteractNPC(const uint32_t npcId, const std::string &option_text) {
		uint8_t data[100] = { 0 };
		*reinterpret_cast<uint32_t*>(&data[0x50]) = npcId;

		dataStruct dt;
		dt.dataPtr = data;


		NPCDef npc_def;
		if (!GetNPCDef(npcId, &npc_def)) {
			return false;
		}

		const std::vector<NPCDef::Option> options = npc_def.options();


		std::vector<NPCDef::Option> filtered;
		std::copy_if(options.begin(), options.end(), std::back_inserter(filtered), [option_text](const NPCDef::Option &opt) {
			return opt.text.compare(option_text) == 0;
		});

		if (filtered.size() == 0) {
			return false;
		}

		const NPCDef::Option &option = filtered[0];

		uint64_t ctx = 0;

		switch (option.index) {
		case 0:
			ctx = actionContexts[OptionId::NPCOption1];
			break;
		case 1:
			ctx = actionContexts[OptionId::NPCAttack];
			break;
		case 2:
			ctx = actionContexts[OptionId::NPCOption2];
			break;
		case 3:
			ctx = actionContexts[OptionId::NPCOption3];
			break;
		case 4:
			ctx = actionContexts[OptionId::NPCOption4];
			break;
		case 5:
			ctx = actionContexts[OptionId::NPCExamine];
			break;
		}

		typedef uint64_t(__cdecl *_GetOptionHandler)(uint64_t ctx);
		const uint64_t option_handler = reinterpret_cast<_GetOptionHandler>(g_gameBase + 0xC3E40)(ctx);
		const uint64_t option_handler_vtable = ReadPtr(option_handler);


		/*std::cout << "ctx: " << std::hex << ctx << std::endl;
		std::cout << "option_handler: " << std::hex << option_handler << std::endl;*/

		uint64_t do_option_func = ReadPtrOffset(option_handler_vtable, 0x10);
		// std::cout << "option_handler_func: " << std::hex << do_option_func << std::endl;

		typedef void(__cdecl *_DoOption)(uint64_t _this, void *dataPtr);
		reinterpret_cast<_DoOption>(do_option_func)(option_handler, &dt);
		return true;
	}

	





	inline void WalkTo(int32_t x, int32_t y) {
		uint8_t data[100] = { 0 };

		*reinterpret_cast<int*>(&data[0x54]) = x;
		*reinterpret_cast<int*>(&data[0x58]) = y;

		uint64_t handler = g_gameBase + 0x6B96E0;
		uint64_t handler_vtable = ReadPtr(handler);
		uint64_t func_ptr = ReadPtrOffset(handler_vtable, 0x10);

		/* std::cout << "ctx: " << std::hex << handler << std::endl;
		std::cout << "walk func_ptr: " << std::hex << func_ptr << std::endl;
		std::cout << "MoveTo: " << x << ", " << y << std::endl; */

		dataStruct dt;
		dt.dataPtr = data;

		typedef void(__cdecl *_MoveTo)(uint64_t _this, void *dataPtr);
		reinterpret_cast<_MoveTo>(func_ptr)(handler, &dt);
	}
}
