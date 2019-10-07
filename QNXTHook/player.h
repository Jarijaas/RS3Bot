#pragma once


/*
      const char *name = (const char *)(entity_ptr + 0x128);
      entity->set_name(name);
      // uint32_t playerId = *reinterpret_cast<uint32_t*>(entity + 0xF8);
      // entity->set_id(playerId);

      int32_t tile_x, tile_y;
      Game::WorldToTilePos(
          *reinterpret_cast<int32_t *>(entity_ptr + 0x39C),
          *reinterpret_cast<int32_t *>(entity_ptr + 0x39C + 16), &tile_x,
          &tile_y);

      uint64_t currentStatePtr =
          *reinterpret_cast<uint32_t *>(entity_ptr + 0x5A8);
      uint64_t interactingStatePtr =
          *reinterpret_cast<uint32_t *>(entity_ptr + 0x5A8 + 8);
      uint64_t movementPtr = *reinterpret_cast<uint64_t *>(entity_ptr + 0x280);

      uint32_t movType = *reinterpret_cast<uint32_t *>(movementPtr);
      uint32_t actionId = *reinterpret_cast<uint32_t *>(entity_ptr + 0x588);*/

#define PLAYER_NAME_OFFSET 0x128

#define CURRENT_STATE_OFFSET 0x5A8
#define ANIMATING_STATE_OFFSET CURRENT_STATE_OFFSET + 0x08

#define MOVEMENT_OFFSET 0x280
#define ACTION_ID_OFFSET 0x588

#define POSITION_X_OFFSET 0x39C
#define POSITION_Y_OFFSET POSITION_X_OFFSET + 0x10

class PlayerEntity {
 public:
  PlayerEntity(const uint64_t base_ptr) : ptr_(base_ptr) {}

  const std::string name() const {
    return std::string(
        reinterpret_cast<const char *>(ptr_ + PLAYER_NAME_OFFSET));
  }

  const uint64_t current_state_ptr() const {
    return *reinterpret_cast<uint64_t *>(ptr_ + CURRENT_STATE_OFFSET);
  }

  const uint64_t animating_state_ptr() const {
    return *reinterpret_cast<uint64_t *>(ptr_ + ANIMATING_STATE_OFFSET);
  }

  const bool IsAnimating() const {
    return current_state_ptr() == animating_state_ptr();
  }

  const uint64_t movement_ptr() const {
    return *reinterpret_cast<uint64_t *>(ptr_ + MOVEMENT_OFFSET);
  }

  const Entity_MovementType movement_type() const {
    return static_cast<Entity_MovementType>(
        *reinterpret_cast<uint32_t *>(movement_ptr()));
  }

  const uint32_t action_id() const {
    return *reinterpret_cast<uint64_t *>(ptr_ + ACTION_ID_OFFSET);
  }

  void GetTilePosition(int32_t *tile_x, int32_t *tile_y) const {
    Game::WorldToTilePos(
        *reinterpret_cast<int32_t *>(ptr_ + POSITION_X_OFFSET),
        *reinterpret_cast<int32_t *>(ptr_ + POSITION_Y_OFFSET), tile_x,
        tile_y);
  }

  void ToGRPCEntity(Entity *out) const {
    out->set_name(name());
    out->set_action_id(action_id());
    out->set_mov_type(movement_type());

    out->set_is_interacting(IsAnimating());
    out->set_type(Player);

	int32_t tile_x, tile_y;
    GetTilePosition(&tile_x, &tile_y);

	Location *loc = out->mutable_location();
	loc->set_tile_x(tile_x);
    loc->set_tile_y(tile_y);
  }

 private:
  const uint64_t ptr_;
};