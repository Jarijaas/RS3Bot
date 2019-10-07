#include "Server.h"

#include <grpc/support/log.h>

#pragma comment(lib, "zlibstatic.lib")
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "libprotoc.lib")
#pragma comment(lib, "grpc_unsecure.lib")
#pragma comment(lib, "grpc++.lib")
#pragma comment(lib, "gpr.lib")

#include <iostream>
#include <memory>
#include <string>
#include "common.h"

#include "player.h"

using namespace std::placeholders;

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using std::cout;
using std::dec;
using std::endl;
using std::hex;

RPCServer::RPCServer() {}

RPCServer::~RPCServer() {
	grpcServer_->Shutdown();
	cq_->Shutdown();
}

ServerBuilder builder;

// extern std::vector<ChatMessage> kRpcDispatchQueue;

grpc::Status HandleFindEntities(const EntityRequest &req,
	EntitiesResponse *response) {
	std::vector<uint64_t> srv_entities;
	Game::GetServerEntities(&srv_entities);

	std::set<uint64_t> static_entities;
	Game::GetStaticEntities(&static_entities);

	std::vector<uint64_t> visible_entities;
	Game::GetVisibleEntities(&visible_entities);

	cout << "srvEntititesCount: " << srv_entities.size() << endl;
	cout << "visibleEntititesCount: " << visible_entities.size() << endl;
	cout << "staticEntititesCount: " << static_entities.size() << endl;

	// cout << "g_gameContext: " << hex << g_gameContext << endl;

	for (const uint64_t &entity_ptr : srv_entities) {
		auto entity = response->add_entities();
		auto loc = entity->mutable_location();

		const char *name = (const char *)(entity_ptr + 0x128);
		entity->set_name(name);
		uint32_t id = *reinterpret_cast<uint32_t *>(entity_ptr + 0xF8);
		// uint32_t pos = *reinterpret_cast<uint32_t*>(entityPtr + 0x104C);

		// cout << "Server Entity " << name << " @ " << hex << entity_ptr << endl;

		// Game::NPCDef def(*reinterpret_cast<uint64_t*>(entityPtr + 0x1010));
		// def.options();

		const uint32_t *attributes =
			reinterpret_cast<uint32_t *>(entity_ptr + 0x101C);

		entity->set_health(attributes[static_cast<int>(NPCAttributes::CurrentHP)]);
		entity->set_max_health(attributes[static_cast<int>(NPCAttributes::MaxHP)]);
		entity->set_level(attributes[static_cast<int>(NPCAttributes::Level)]);

		int32_t tile_x, tile_y;
		Game::WorldToTilePos(
			*reinterpret_cast<int32_t *>(entity_ptr + 0x39C),
			*reinterpret_cast<int32_t *>(entity_ptr + 0x39C + 0x10), &tile_x,
			&tile_y);

		const uint64_t curr_state_ptr =
			*reinterpret_cast<uint32_t *>(entity_ptr + 0x5A8);
		const uint64_t interacting_state_ptr =
			*reinterpret_cast<uint32_t *>(entity_ptr + 0x5A8 + 8);

		const uint64_t movement_ptr =
			*reinterpret_cast<uint64_t *>(entity_ptr + 0x280);
		const uint32_t mov_type = *reinterpret_cast<uint32_t *>(movement_ptr);

		entity->set_is_interacting(curr_state_ptr == interacting_state_ptr);
		entity->set_mov_type(static_cast<Entity_MovementType>(mov_type));

		entity->set_id(id);
		loc->set_tile_x(tile_x);
		loc->set_tile_y(tile_y);
		entity->set_type(EntityType::NPC);
	}

	/*for (const uint64_t &entity_ptr : visible_entities) {
			EntityType type = *reinterpret_cast<EntityType*>(entity_ptr + 0x40);
			if (type == EntityType::GroundItem) {
					std::vector<Game::ItemDef> defs;
					Game::GetItemDefsOnTile(entity_ptr, &defs);

					// CursorGroundItemOptions
					// movss   xmm1, dword ptr [rdi+974h]
					// movss   xmm1, dword ptr [rdi+97Ch]

					const int32_t tile_x =
	floor(*reinterpret_cast<float*>(entity_ptr + 0x974) / 512.0f); const int32_t
	tile_y = floor(*reinterpret_cast<float*>(entity_ptr + 0x974 + 8) / 512.0f);

					for (const Game::ItemDef &item : defs) {
							auto entity = response->add_entities();

							entity->set_id(item.id());

							auto loc = entity->mutable_location();
							loc->set_tile_x(tile_x);
							loc->set_tile_y(tile_y);

							entity->set_name(item.name());

							// cout << "GroundItem: " << item.name().c_str() << "
	@ " << hex << entity_ptr << ", objDef: " << item.base() << endl;
							entity->set_type(type);
					}
			} else if (type == EntityType::Object) {
					uint64_t def_ptr = *reinterpret_cast<uint64_t*>(entity_ptr +
	0x110 + 8); if (def_ptr == 0) { continue;
					}

					Game::ObjectDef def(def_ptr);

					auto entity = response->add_entities();
					auto loc = entity->mutable_location();

					entity->set_id(def.id());
					entity->set_name(def.name());

					int32_t tile_x = *reinterpret_cast<int32_t*>(entity_ptr +
	0x138 + 4); int32_t tile_y = *reinterpret_cast<int32_t*>(entity_ptr + 0x140);

					// cout << "Object1V " << def.name() << " @ " << hex <<
	entity_ptr << ", objDef: " << def_ptr << dec << ", " << tile_x << ", " <<
	tile_y << endl;

					loc->set_tile_x(tile_x);
					loc->set_tile_y(tile_y);
					entity->set_type(type);
			}
	}*/

	const uint64_t controlled_player = Game::GetControlledPlayer();

	for (const uint64_t &entity_ptr : static_entities) {
		EntityType type = *reinterpret_cast<EntityType *>(entity_ptr + 0x40);

		if (type == EntityType::Player) {
			auto entity = response->add_entities();

			const PlayerEntity player(entity_ptr);
			player.ToGRPCEntity(entity);

			entity->set_is_controlled(entity_ptr == controlled_player);

			// cout << "Player " << name << " @ " << hex << entity_ptr << ",
			// is_controlled: " << is_controlled << endl;
			/*cout << "103Ch: " << hex << *(uint32_t*)(entity_ptr + 0x103C) << endl;
			cout << "1040h: " << hex << *(uint32_t*)(entity_ptr + 0x1040) << endl;
			cout << "1044h: " << hex << *(uint32_t*)(entity_ptr + 0x1044) << endl;
			cout << "1048h: " << hex << *(uint32_t*)(entity_ptr + 0x1048) << endl;*/
			// cout << "x: " << dec << tile_x << ", y: " << dec << tile_y << endl;
		}
		else if (type == EntityType::Object2) {
			uint64_t def_ptr = *reinterpret_cast<uint64_t *>(entity_ptr + 0xF8 + 8);
			if (def_ptr == 0) {
				continue;
			}

			auto entity = response->add_entities();
			auto loc = entity->mutable_location();

			const Game::ObjectDef def(def_ptr);

			entity->set_id(def.id());
			entity->set_name(def.name());

			int32_t tile_x = *reinterpret_cast<uint32_t *>(entity_ptr + 0xE8 + 4);
			int32_t tile_y = *reinterpret_cast<uint32_t *>(entity_ptr + 0xF0);
			// cout << "Object2 " << def.name() << " @ " << hex << entity_ptr << ",
			// objDef: " << def_ptr << dec << ", " << tile_x << ", " << tile_y <<
			// endl;

			loc->set_tile_x(tile_x);
			loc->set_tile_y(tile_y);
			entity->set_type(type);
		}
		else if (type == EntityType::Object) {
			uint64_t def_ptr = *reinterpret_cast<uint64_t *>(entity_ptr + 0x110 + 8);
			if (def_ptr == 0) {
				continue;
			}

			const Game::ObjectDef def(def_ptr);

			auto entity = response->add_entities();
			auto loc = entity->mutable_location();

			entity->set_id(def.id());
			entity->set_name(def.name());

			int32_t tile_x = *reinterpret_cast<int32_t *>(entity_ptr + 0x138 + 4);
			int32_t tile_y = *reinterpret_cast<int32_t *>(entity_ptr + 0x140);

			// cout << "Object1 " << def.name() << " @ " << hex << entity_ptr << ",
			// objDef: " << def_ptr << dec << ", " << tile_x << ", " << tile_y <<
			// endl;

			loc->set_tile_x(tile_x);
			loc->set_tile_y(tile_y);
			entity->set_type(type);
		}
		else if (type == EntityType::GroundItem) {
			std::vector<Game::ItemDef> defs;
			Game::GetItemDefsOnTile(entity_ptr, &defs);

			// CursorGroundItemOptions
			// movss   xmm1, dword ptr [rdi+974h]
			// movss   xmm1, dword ptr [rdi+97Ch]

			const int32_t tile_x =
				floor(*reinterpret_cast<float *>(entity_ptr + 0x974) / 512.0f);
			const int32_t tile_y =
				floor(*reinterpret_cast<float *>(entity_ptr + 0x974 + 8) / 512.0f);

			for (const Game::ItemDef &item : defs) {
				auto entity = response->add_entities();

				entity->set_id(item.id());

				auto loc = entity->mutable_location();
				loc->set_tile_x(tile_x);
				loc->set_tile_y(tile_y);

				entity->set_name(item.name());

				// cout << "GroundItem: " << item.name().c_str() << " @ " << hex <<
				// entity_ptr << ", objDef: " << item.base() << endl;
				entity->set_type(type);
			}
		}
		else {
			// 6, 8, 9, 10, 11, 15
			// cout << "unk static type: " << type << endl;
		}
	}
	return grpc::Status::OK;
}

grpc::Status HandleGetInventory(const InventoryTypeRequest &req,
	Inventory *response) {
	auto inventory = Game::GetInventory(req.type());

	// cout << "base: " << hex << inventory.base_ptr() << endl;

	int slot = 0;
	for (const Game::InventoryItem &item : inventory) {
		if (item.id != -1) {
			auto replyItem = response->add_items();
			replyItem->set_id(item.id);
			replyItem->set_quantity(item.quantity);

			const Game::ItemDef item_def = Game::GetItemDef(item.id);

			replyItem->set_name(item_def.name());
			replyItem->set_slot(slot);
		}
		slot++;
	}

	return grpc::Status::OK;
}


void RPCServer::Run() {
	const DWORD proc_id = GetCurrentProcessId();

	// const std::string addr_uri = "127." + std::to_string(proc_id & 0xFF) + "."
	// + std::to_string(proc_id >> 8) + ".2:50051";
	const std::string addr_uri = "127.0.0.1:50051";

	builder.AddListeningPort(addr_uri, grpc::InsecureServerCredentials());

	builder.RegisterService(&asyncService_);
	cq_ = builder.AddCompletionQueue();
	grpcServer_ = builder.BuildAndStart();

	cout << "grpc server is listening at " << addr_uri << endl;

	// Spawn handlers for messages to serve new clients.

	new RequestHandler<EntityRequest, EntitiesResponse>(
		&asyncService_, cq_.get(), &RS3::AsyncService::RequestFindEntities,
		HandleFindEntities);

	new RequestHandler<InteractRequest>(
		&asyncService_, cq_.get(), &RS3::AsyncService::RequestInteract,
		[](const InteractRequest &req, Empty *) -> grpc::Status {
		DoAntiAFK();

		const Entity &entity = req.entity();
		const Location &loc = entity.location();
		const EntityType &type = entity.type();

		bool res = false;

		if (type == EntityType::Object || type == EntityType::Object2) {
			res = Game::InteractObject(entity.id(), loc.tile_x(), loc.tile_y(),
				req.option());
		}
		else if (type == EntityType::GroundItem) {
			res = Game::InteractGroundItem(entity.id(), loc.tile_x(),
				loc.tile_y(), req.option());
		}
		else {
			res = Game::InteractNPC(entity.id(), req.option());
		}
		return res ? grpc::Status::OK
			: grpc::Status(
				grpc::StatusCode::NOT_FOUND,
				"Entity does not have " + req.option() + " option");
	});

	new RequestHandler<InventoryTypeRequest, Inventory>(
		&asyncService_, cq_.get(), &RS3::AsyncService::RequestGetInventory,
		HandleGetInventory);

	new RequestHandler<UIAction>(
		&asyncService_, cq_.get(), &RS3::AsyncService::RequestDoUIAction,
		[](const UIAction &req, Empty *) -> grpc::Status {
		DoAntiAFK();

		switch (req.option_case()) {
		case UIAction::OptionCase::kOptionId:
			Game::DoUIAction(req.optionid(), req.param2(), req.id());
			return grpc::Status::OK;
		case UIAction::OptionCase::kOptionText:
			return Game::DoUIAction(req.optiontext(), req.param2(), req.id()) ?
				grpc::Status::OK : grpc::Status(grpc::StatusCode::NOT_FOUND,
					"Widget does not have " + req.optiontext() + " option");
		}

		return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
			"This handler does not support selected option type");
	});

	new RequestHandler<UIAction>(
		&asyncService_, cq_.get(),
		&RS3::AsyncService::RequestDoItemInterfaceAction,
		[](const UIAction &req, Empty *) -> grpc::Status {
		Game::DoItemInterfaceAction(req.optionid(), req.param2(), req.id());
		return grpc::Status::OK;
	});

	new RequestHandler<ItemAction>(
		&asyncService_, cq_.get(), &RS3::AsyncService::RequestBackpackItemAction,
		[](const ItemAction req, Empty *) -> grpc::Status {
		DoAntiAFK();

		return Game::BackpackInteractItem(req.slot(), req.option())
			? grpc::Status::OK
			: grpc::Status(
				grpc::StatusCode::NOT_FOUND,
				"Item does not have " + req.option() + " option");
	});

	new RequestHandler<Location>(
		&asyncService_, cq_.get(), &RS3::AsyncService::RequestMoveTo,
		[](const Location &req, Empty *) -> grpc::Status {
		DoAntiAFK();
		Game::WalkTo(req.tile_x(), req.tile_y());
		return grpc::Status::OK;
	});

	new RequestHandler<IdRequest, PropertyValue>(
		&asyncService_, cq_.get(), &RS3::AsyncService::RequestGetProperty,
		[](const IdRequest &req, PropertyValue *response) -> grpc::Status {
		uint32_t value = 0;
		if (Game::GetProperty(req.id(), &value)) {
			response->set_value(value);
			return Status::OK;
		}
		return Status(grpc::StatusCode::NOT_FOUND, "Could not find a property");
	});

	new RequestHandler<IdRequest, BoolResponse>(
		&asyncService_, cq_.get(), &RS3::AsyncService::RequestIsUIWidgetVisible,
		[](const IdRequest &req, BoolResponse *response) -> grpc::Status {
		uint64_t widget_ptr = 0;
		cout << "Get UI Widget: " << hex << req.id() << endl;
		bool visible = Game::GetUIWidget(req.id(), &widget_ptr);
		response->set_value(visible);
		return Status::OK;
	});

	new RequestHandler<Empty, Entity>(
		&asyncService_, cq_.get(),
		&RS3::AsyncService::RequestGetControlledPlayer,
		[](const Empty &, Entity *response) -> grpc::Status {
		const uint64_t controlled_ptr = Game::GetControlledPlayer();

		const PlayerEntity player(controlled_ptr);
		player.ToGRPCEntity(response);
		response->set_is_controlled(true);

		return Status::OK;
	});

	new RequestHandler<IdRequest, Widget>(
		&asyncService_, cq_.get(),
		&RS3::AsyncService::RequestGetWidget,
		[](const IdRequest &req, Widget *response) -> grpc::Status {

		Game::UIWidgetDef child_def;
		if (!Game::GetUIWidgetChild(req.id(), &child_def)) {
			return Status(grpc::StatusCode::NOT_FOUND, "Could not find a widget");
		}

		for (const Game::UIWidgetDef::Option &option : child_def.options()) {
			response->add_options(option.text);
		}

		response->set_widgetid(req.id());
		response->set_name(child_def.name());

		return Status::OK;
	});

	/*new StreamRequestHandler<Empty, ChatMessage>(
		this, &asyncService_, cq_.get(),
		&RS3::AsyncService::RequestOnChatMessage,
		[](RPCServer *server, const Empty &req,
			StreamRequestHandler<Empty, ChatMessage> *writer) -> CompletionState {

		auto msgs = server->GetEvents<ChatMessage>();
		if (!msgs.empty()) {
			writer->Write(*std::static_pointer_cast<ChatMessage>(msgs.front()));
			return CompletionState::COMPLETED;
		}

		return CompletionState::WAITING;
	});*/

	/*new StreamRequestHandler<Empty, InterfaceOpenMessage>(
		this, &asyncService, cq_.get(),
		&RS3::AsyncService::RequestOnInterfaceOpen,
		[](RPCServer *server, const Empty &req,
			StreamRequestHandler<Empty, InterfaceOpenMessage> *writer) -> CompletionState {

		auto msgs = server->GetEvents<InterfaceOpenMessage>();
		if (!msgs.empty()) {
			writer->Write(*std::static_pointer_cast<InterfaceOpenMessage>(msgs.front()));
			return CompletionState::COMPLETED;
		}

		return CompletionState::WAITING;
	});*/

	// NewGenericStreamConsumer<ChatMessage>(&RS3::AsyncService::RequestOnChatMessage);
	// NewGenericStreamConsumer<InterfaceOpenMessage>(&RS3::AsyncService::RequestOnInterfaceOpen);
	// NewGenericStreamConsumer<CursorActionMessage>(&RS3::AsyncService::RequestOnCursorAction);
}

void RPCServer::Handle() {
	// Block waiting to read the next event from the completion queue. The
	// event is uniquely identified by its tag, which in this case is the
	// memory address of a CallData instance.
	// The return value of Next should always be checked. This return value
	// tells us whether there is any kind of event or cq_ is shutting down.

	std::lock_guard<std::mutex> guard(eventsMutex_);

	void *tag;  // Uniquely identifies a request.
	bool ok;

	// Process requests until deadline
	const auto deadline =
		std::chrono::system_clock::now() + std::chrono::milliseconds(10);

	const ServerCompletionQueue::NextStatus status =
		cq_->AsyncNext(&tag, &ok, deadline);
	if (tag != nullptr) {
		auto req = static_cast<RequestHandlerBase *>(tag);

		if (status == ServerCompletionQueue::NextStatus::GOT_EVENT) {
			if (ok) {
				req->Proceed();

				// If the stream waits for data add to another container,
				// because CompletionQueue removes the request from the queue after
				// AsyncNext has been called
				if (req->State() == CompletionState::WAITING) {
					waitingStreams_.push_back(req);
				}
			}
			else {
				// Ok is false when something is went wrong with an operation
				// For example, happens when writing on a closed request,
				// which is used here to trigger streaming request cleanup
				if (req->IsStreamingRequest()) {
					delete req;
				}
			}
		}
	}

	// Process waiting streams
	for (auto it = waitingStreams_.begin(); it != waitingStreams_.end();) {
		(*it)->Proceed();

		// Remove if the stream is not waiting anymore
		if ((*it)->State() != CompletionState::WAITING) {
			waitingStreams_.erase(it);
		}
		else {
			++it;
		}
	}

	// Remove processed events
	for (auto &n : events_) {
		if (!n.second.empty()) {
			n.second.pop();
		}
	}
}
