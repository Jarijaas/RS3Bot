var exports = exports || {};
var ByteBuffer = ByteBuffer || require("bytebuffer");
exports.Long = ByteBuffer.Long;

(function(undefined) {

  function pushTemporaryLength(buffer) {
    var length = buffer.readVarint32();
    var limit = buffer.limit;
    buffer.limit = buffer.offset + length;
    return limit;
  }

  function skipUnknownField(buffer, type) {
    switch (type) {
      case 0: while (buffer.readByte() & 0x80) {} break;
      case 2: buffer.skip(buffer.readVarint32()); break;
      case 5: buffer.skip(4); break;
      case 1: buffer.skip(8); break;
      default: throw new Error("Unimplemented type: " + type);
    }
  }

  function coerceLong(value) {
    if (!(value instanceof ByteBuffer.Long) && "low" in value && "high" in value)
      value = new ByteBuffer.Long(value.low, value.high, value.unsigned);
    return value;
  }

  exports["encodePropertyType"] = {
    "UnknownPropertyType": 0,
    "Health": 659
  };

  exports["decodePropertyType"] = {
    "0": "UnknownPropertyType",
    "659": "Health"
  };

  exports["encodeUIWidgetType"] = {
    "UnknownWidget": 0,
    "BankWithdrawItem": 49938656,
    "BankDepositItem": 49938459,
    "BankTabAll": 49938560,
    "BankTab2": 49938568,
    "BankTab3": 49938576,
    "BankTab4": 49938584,
    "BankTab5": 49938592,
    "BankTab6": 49938600,
    "BankTab7": 49938608,
    "BankTab8": 49938616,
    "BankTab9": 49938624,
    "LobbySelectTab": 59375714,
    "LobbyPlayNow": 59375747,
    "SelectWorld": 59637841,
    "ChatOptionNext": 77594639,
    "ChatOption1": 77856773,
    "ChatOption2": 77856778,
    "ChatOption3": 77856783,
    "ChatOption4": 77856788,
    "ChatOption5": 77856793,
    "Regenerate": 93716491,
    "Retaliate": 93716535,
    "QuickPrayers": 93716496,
    "FollowerSelectLeftClickOption": 93716503,
    "FollowerAttack": 93716509,
    "InteractFollower": 93716510,
    "DismissFollower": 93716525,
    "CallFollower": 93716526,
    "SummonPet": 93716529,
    "SkillsItem": 96075783,
    "WorldMap": 96010249,
    "RunEnergy": 96010251,
    "HomeTeleport": 96010258,
    "Compass": 125763586,
    "BackpackItem": 96534533,
    "DepositAll": 49938523,
    "DepositAllBox": 720907,
    "ActionBar1": 93716735,
    "ActionBar2": 93716736,
    "ActionBar3": 93716737,
    "ActionBar4": 93716738,
    "ActionBarSlot1": 93716542,
    "ActionBarSlot2": 93716555,
    "ActionBarSlot3": 93716568,
    "ActionBarSlot4": 93716581,
    "ActionBarSlot5": 93716594,
    "ActionBarSlot6": 93716607,
    "ActionBarSlot7": 93716620,
    "ActionBarSlot8": 93716633,
    "ActionBarSlot9": 93716646,
    "ActionBarSlot10": 93716659,
    "ActionBarSlot11": 93716672,
    "ActionBarSlot12": 93716685,
    "ActionBarSlot13": 93716698,
    "ActionBarSlot14": 93716711,
    "ActionBarSlot15": 93716724,
    "Settings": 93913143,
    "ExitToLobby": 93913151,
    "ChatEnterMessage": 8978506,
    "BanditCamp": 71565320,
    "LunarIsle": 71565321,
    "AlKharid": 71565322,
    "Ardougne": 71565323,
    "Burthorpe": 71565324,
    "Catherby": 71565325,
    "DraynorVillage": 71565326,
    "Edgeville": 71565327,
    "Falador": 71565328,
    "Lumbdridge": 71565329,
    "PortSarim": 71565330,
    "SeersVillage": 71565331,
    "Taverley": 71565332,
    "Varrock": 71565333,
    "Menaphos": 71565334,
    "Yanille": 71565335,
    "Canifis": 71565336,
    "EaglesPeak": 71565337,
    "FremennikProvince": 71565338,
    "Karamja": 71565339,
    "Ooglog": 71565340,
    "Tirannw": 71565341,
    "Wilderness": 71565342,
    "Ashdale": 71565343,
    "Priffdinas": 71565344
  };

  exports["decodeUIWidgetType"] = {
    "0": "UnknownWidget",
    "720907": "DepositAllBox",
    "8978506": "ChatEnterMessage",
    "49938459": "BankDepositItem",
    "49938523": "DepositAll",
    "49938560": "BankTabAll",
    "49938568": "BankTab2",
    "49938576": "BankTab3",
    "49938584": "BankTab4",
    "49938592": "BankTab5",
    "49938600": "BankTab6",
    "49938608": "BankTab7",
    "49938616": "BankTab8",
    "49938624": "BankTab9",
    "49938656": "BankWithdrawItem",
    "59375714": "LobbySelectTab",
    "59375747": "LobbyPlayNow",
    "59637841": "SelectWorld",
    "71565320": "BanditCamp",
    "71565321": "LunarIsle",
    "71565322": "AlKharid",
    "71565323": "Ardougne",
    "71565324": "Burthorpe",
    "71565325": "Catherby",
    "71565326": "DraynorVillage",
    "71565327": "Edgeville",
    "71565328": "Falador",
    "71565329": "Lumbdridge",
    "71565330": "PortSarim",
    "71565331": "SeersVillage",
    "71565332": "Taverley",
    "71565333": "Varrock",
    "71565334": "Menaphos",
    "71565335": "Yanille",
    "71565336": "Canifis",
    "71565337": "EaglesPeak",
    "71565338": "FremennikProvince",
    "71565339": "Karamja",
    "71565340": "Ooglog",
    "71565341": "Tirannw",
    "71565342": "Wilderness",
    "71565343": "Ashdale",
    "71565344": "Priffdinas",
    "77594639": "ChatOptionNext",
    "77856773": "ChatOption1",
    "77856778": "ChatOption2",
    "77856783": "ChatOption3",
    "77856788": "ChatOption4",
    "77856793": "ChatOption5",
    "93716491": "Regenerate",
    "93716496": "QuickPrayers",
    "93716503": "FollowerSelectLeftClickOption",
    "93716509": "FollowerAttack",
    "93716510": "InteractFollower",
    "93716525": "DismissFollower",
    "93716526": "CallFollower",
    "93716529": "SummonPet",
    "93716535": "Retaliate",
    "93716542": "ActionBarSlot1",
    "93716555": "ActionBarSlot2",
    "93716568": "ActionBarSlot3",
    "93716581": "ActionBarSlot4",
    "93716594": "ActionBarSlot5",
    "93716607": "ActionBarSlot6",
    "93716620": "ActionBarSlot7",
    "93716633": "ActionBarSlot8",
    "93716646": "ActionBarSlot9",
    "93716659": "ActionBarSlot10",
    "93716672": "ActionBarSlot11",
    "93716685": "ActionBarSlot12",
    "93716698": "ActionBarSlot13",
    "93716711": "ActionBarSlot14",
    "93716724": "ActionBarSlot15",
    "93716735": "ActionBar1",
    "93716736": "ActionBar2",
    "93716737": "ActionBar3",
    "93716738": "ActionBar4",
    "93913143": "Settings",
    "93913151": "ExitToLobby",
    "96010249": "WorldMap",
    "96010251": "RunEnergy",
    "96010258": "HomeTeleport",
    "96075783": "SkillsItem",
    "96534533": "BackpackItem",
    "125763586": "Compass"
  };

  exports["encodeEntityType"] = {
    "Object": 0,
    "NPC": 1,
    "Player": 2,
    "GroundItem": 3,
    "Object2": 12
  };

  exports["decodeEntityType"] = {
    "0": "Object",
    "1": "NPC",
    "2": "Player",
    "3": "GroundItem",
    "12": "Object2"
  };

  exports["encodeIdRequest"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional uint32 id = 1;
    var value = message["id"];
    if (value !== undefined) {
      buffer.writeVarint32(8);
      buffer.writeVarint32(value);
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeIdRequest"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional uint32 id = 1;
      case 1:
        message["id"] = buffer.readVarint32() >>> 0;
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeBoolResponse"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional bool value = 1;
    var value = message["value"];
    if (value !== undefined) {
      buffer.writeVarint32(8);
      buffer.writeByte(value ? 1 : 0);
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeBoolResponse"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional bool value = 1;
      case 1:
        message["value"] = !!buffer.readByte();
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeInventoryTypeRequest"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional uint32 type = 1;
    var value = message["type"];
    if (value !== undefined) {
      buffer.writeVarint32(8);
      buffer.writeVarint32(value);
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeInventoryTypeRequest"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional uint32 type = 1;
      case 1:
        message["type"] = buffer.readVarint32() >>> 0;
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodePropertyValue"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional uint32 value = 1;
    var value = message["value"];
    if (value !== undefined) {
      buffer.writeVarint32(8);
      buffer.writeVarint32(value);
    }

    return buffer.flip().toBuffer();
  };

  exports["decodePropertyValue"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional uint32 value = 1;
      case 1:
        message["value"] = buffer.readVarint32() >>> 0;
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeEntityRequest"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional uint32 id = 1;
    var value = message["id"];
    if (value !== undefined) {
      buffer.writeVarint32(8);
      buffer.writeVarint32(value);
    }

    // optional string name = 2;
    var value = message["name"];
    if (value !== undefined) {
      buffer.writeVarint32(18);
      var nested = new ByteBuffer(undefined, true);
      nested.writeUTF8String(value), buffer.writeVarint32(nested.flip().limit), buffer.append(nested);
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeEntityRequest"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional uint32 id = 1;
      case 1:
        message["id"] = buffer.readVarint32() >>> 0;
        break;

      // optional string name = 2;
      case 2:
        message["name"] = buffer.readUTF8String(buffer.readVarint32(), "b");
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeUIAction"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional int32 param1 = 1;
    var value = message["param1"];
    if (value !== undefined) {
      buffer.writeVarint32(8);
      buffer.writeVarint64(value | 0);
    }

    // optional int32 param2 = 2;
    var value = message["param2"];
    if (value !== undefined) {
      buffer.writeVarint32(16);
      buffer.writeVarint64(value | 0);
    }

    // optional UIWidgetType id = 3;
    var value = message["id"];
    if (value !== undefined) {
      buffer.writeVarint32(24);
      buffer.writeVarint32(exports["encodeUIWidgetType"][value]);
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeUIAction"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional int32 param1 = 1;
      case 1:
        message["param1"] = buffer.readVarint32();
        break;

      // optional int32 param2 = 2;
      case 2:
        message["param2"] = buffer.readVarint32();
        break;

      // optional UIWidgetType id = 3;
      case 3:
        message["id"] = exports["decodeUIWidgetType"][buffer.readVarint32()];
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeItemAction"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional uint32 slot = 1;
    var value = message["slot"];
    if (value !== undefined) {
      buffer.writeVarint32(8);
      buffer.writeVarint32(value);
    }

    // optional string option = 2;
    var value = message["option"];
    if (value !== undefined) {
      buffer.writeVarint32(18);
      var nested = new ByteBuffer(undefined, true);
      nested.writeUTF8String(value), buffer.writeVarint32(nested.flip().limit), buffer.append(nested);
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeItemAction"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional uint32 slot = 1;
      case 1:
        message["slot"] = buffer.readVarint32() >>> 0;
        break;

      // optional string option = 2;
      case 2:
        message["option"] = buffer.readUTF8String(buffer.readVarint32(), "b");
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeEmpty"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    return buffer.flip().toBuffer();
  };

  exports["decodeEmpty"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeEntity"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional uint32 id = 1;
    var value = message["id"];
    if (value !== undefined) {
      buffer.writeVarint32(8);
      buffer.writeVarint32(value);
    }

    // optional EntityType type = 2;
    var value = message["type"];
    if (value !== undefined) {
      buffer.writeVarint32(16);
      buffer.writeVarint32(exports["encodeEntityType"][value]);
    }

    // optional string name = 3;
    var value = message["name"];
    if (value !== undefined) {
      buffer.writeVarint32(26);
      var nested = new ByteBuffer(undefined, true);
      nested.writeUTF8String(value), buffer.writeVarint32(nested.flip().limit), buffer.append(nested);
    }

    // optional Location location = 4;
    var value = message["location"];
    if (value !== undefined) {
      buffer.writeVarint32(34);
      var nested = exports["encodeLocation"](value);
      buffer.writeVarint32(nested.byteLength), buffer.append(nested);
    }

    // optional int32 health = 5;
    var value = message["health"];
    if (value !== undefined) {
      buffer.writeVarint32(40);
      buffer.writeVarint64(value | 0);
    }

    // optional int32 max_health = 6;
    var value = message["max_health"];
    if (value !== undefined) {
      buffer.writeVarint32(48);
      buffer.writeVarint64(value | 0);
    }

    // optional int32 level = 7;
    var value = message["level"];
    if (value !== undefined) {
      buffer.writeVarint32(56);
      buffer.writeVarint64(value | 0);
    }

    // optional bool is_interacting = 8;
    var value = message["is_interacting"];
    if (value !== undefined) {
      buffer.writeVarint32(64);
      buffer.writeByte(value ? 1 : 0);
    }

    // optional MovementType mov_type = 9;
    var value = message["mov_type"];
    if (value !== undefined) {
      buffer.writeVarint32(74);
      var nested = exports["encodeMovementType"](value);
      buffer.writeVarint32(nested.byteLength), buffer.append(nested);
    }

    // optional uint32 action_id = 10;
    var value = message["action_id"];
    if (value !== undefined) {
      buffer.writeVarint32(80);
      buffer.writeVarint32(value);
    }

    // repeated string options = 11;
    var values = message["options"];
    if (values !== undefined) {
      for (var i = 0; i < values.length; i++) {
        var value = values[i];
        var nested = new ByteBuffer(undefined, true);
        buffer.writeVarint32(90);
        nested.writeUTF8String(value), buffer.writeVarint32(nested.flip().limit), buffer.append(nested);
      }
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeEntity"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional uint32 id = 1;
      case 1:
        message["id"] = buffer.readVarint32() >>> 0;
        break;

      // optional EntityType type = 2;
      case 2:
        message["type"] = exports["decodeEntityType"][buffer.readVarint32()];
        break;

      // optional string name = 3;
      case 3:
        message["name"] = buffer.readUTF8String(buffer.readVarint32(), "b");
        break;

      // optional Location location = 4;
      case 4:
        var limit = pushTemporaryLength(buffer);
        message["location"] = exports["decodeLocation"](buffer);
        buffer.limit = limit;
        break;

      // optional int32 health = 5;
      case 5:
        message["health"] = buffer.readVarint32();
        break;

      // optional int32 max_health = 6;
      case 6:
        message["max_health"] = buffer.readVarint32();
        break;

      // optional int32 level = 7;
      case 7:
        message["level"] = buffer.readVarint32();
        break;

      // optional bool is_interacting = 8;
      case 8:
        message["is_interacting"] = !!buffer.readByte();
        break;

      // optional MovementType mov_type = 9;
      case 9:
        var limit = pushTemporaryLength(buffer);
        message["mov_type"] = exports["decodeMovementType"](buffer);
        buffer.limit = limit;
        break;

      // optional uint32 action_id = 10;
      case 10:
        message["action_id"] = buffer.readVarint32() >>> 0;
        break;

      // repeated string options = 11;
      case 11:
        var values = message["options"] || (message["options"] = []);
        values.push(buffer.readUTF8String(buffer.readVarint32(), "b"));
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeInteractRequest"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional Entity entity = 1;
    var value = message["entity"];
    if (value !== undefined) {
      buffer.writeVarint32(10);
      var nested = exports["encodeEntity"](value);
      buffer.writeVarint32(nested.byteLength), buffer.append(nested);
    }

    // optional string option = 2;
    var value = message["option"];
    if (value !== undefined) {
      buffer.writeVarint32(18);
      var nested = new ByteBuffer(undefined, true);
      nested.writeUTF8String(value), buffer.writeVarint32(nested.flip().limit), buffer.append(nested);
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeInteractRequest"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional Entity entity = 1;
      case 1:
        var limit = pushTemporaryLength(buffer);
        message["entity"] = exports["decodeEntity"](buffer);
        buffer.limit = limit;
        break;

      // optional string option = 2;
      case 2:
        message["option"] = buffer.readUTF8String(buffer.readVarint32(), "b");
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeEntitiesResponse"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // repeated Entity entities = 1;
    var values = message["entities"];
    if (values !== undefined) {
      for (var i = 0; i < values.length; i++) {
        var value = values[i];
        var nested = exports["encodeEntity"](value);
        buffer.writeVarint32(10);
        buffer.writeVarint32(nested.byteLength), buffer.append(nested);
      }
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeEntitiesResponse"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // repeated Entity entities = 1;
      case 1:
        var limit = pushTemporaryLength(buffer);
        var values = message["entities"] || (message["entities"] = []);
        values.push(exports["decodeEntity"](buffer));
        buffer.limit = limit;
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeLocation"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // optional int32 tile_x = 1;
    var value = message["tile_x"];
    if (value !== undefined) {
      buffer.writeVarint32(8);
      buffer.writeVarint64(value | 0);
    }

    // optional int32 tile_y = 2;
    var value = message["tile_y"];
    if (value !== undefined) {
      buffer.writeVarint32(16);
      buffer.writeVarint64(value | 0);
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeLocation"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // optional int32 tile_x = 1;
      case 1:
        message["tile_x"] = buffer.readVarint32();
        break;

      // optional int32 tile_y = 2;
      case 2:
        message["tile_y"] = buffer.readVarint32();
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

  exports["encodeInventory"] = function(message) {
    var buffer = new ByteBuffer(undefined, true);

    // repeated InventoryItem items = 1;
    var values = message["items"];
    if (values !== undefined) {
      for (var i = 0; i < values.length; i++) {
        var value = values[i];
        var nested = exports["encodeInventoryItem"](value);
        buffer.writeVarint32(10);
        buffer.writeVarint32(nested.byteLength), buffer.append(nested);
      }
    }

    return buffer.flip().toBuffer();
  };

  exports["decodeInventory"] = function(buffer) {
    var message = {};

    if (!(buffer instanceof ByteBuffer))
      buffer = new ByteBuffer.fromBinary(buffer, true);

    end_of_message: while (buffer.remaining() > 0) {
      var tag = buffer.readVarint32();

      switch (tag >>> 3) {
      case 0:
        break end_of_message;

      // repeated InventoryItem items = 1;
      case 1:
        var limit = pushTemporaryLength(buffer);
        var values = message["items"] || (message["items"] = []);
        values.push(exports["decodeInventoryItem"](buffer));
        buffer.limit = limit;
        break;

      default:
        skipUnknownField(buffer, tag & 7);
      }
    }

    return message;
  };

})();
