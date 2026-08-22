#include "antibox/core/antibox.h"
#include "entities.h"
#include "world.h"
#include <vector>

using namespace antibox;

enum ShipState { Hovering, Landed, In_Transit, In_Space };

struct PlayerShip {
  float money = 0.f;
  float debt = 0.f;

  // fuel in liters
  float fuel = 0.f;
  float fuelCapacity = 0.f;
  bool thrusters = false;

  float shipHealth = 0.f;
  float shipHealthMax = 100.f;
  float cargoCapacity = 0.f;
  int currentLocation = 0;
  int currentDestination = 1;
  float errorMessageLength = 0;
  float arrivalTime = 0.f;

  ShipState curState = ShipState::In_Space;

  bool onShip = true;
  bool mainPower = false;

  // cosmetic things - less important
  bool landingGear = false;
  bool cabinLights = false;
  bool wipers = false;

  bool showShipErrorMessage = false;
  std::string shipErrorMessage = "NO FUEL";

  std::vector<CargoStack> cargo;

  std::vector<std::vector<int>> currentMap = {{}};

  std::map<std::string, std::string> sound_effects = {
      {"thrusters_start", "dat/sfxs/engine_startup.wav"},
      {"thrusters_idle", "dat/sfxs/engines_idle.wav"},
      {"thrusters_shutdown", "dat/sfxs/engine_shutdown.wav"},
      {"powered_idle", "dat/sfxs/ambient_on.mp3"},
      {"power_up", "dat/sfxs/startup_ship.wav"},
      {"power_down", "dat/sfxs/power_down.wav"},
      {"terminal_error", "dat/sfxs/alert.wav"},
      {"terminal_type", "dat/sfxs/terminal_type.mp3"},
      {"sam_power_on", "dat/sfxs/sam/sam_power_on.wav"},
      {"sam_thrusters_on", "dat/sfxs/sam/sam_thrusters_on.wav"},
      {"sam_thrusters_off", "dat/sfxs/sam/sam_thrusters_off.wav"},
      {"sam_incoming_comms", "dat/sfxs/sam/sam_incoming_comms.wav"},
      {"flight_sound", "dat/sfxs/in_flight.wav"},
      {"end_flight", "dat/sfxs/rumbling_end.wav"},
      {"low_beep", "dat/sfxs/beep.wav"}};

  void PowerDown() {
    landingGear = false;
    cabinLights = false;
    wipers = false;
    if (thrusters)
      ToggleThrusters();
  }

  void PowerUp() {
    if (!mainPower) {
      Audio::Play(sound_effects["power_up"]);
      Audio::PlayLoop(sound_effects["powered_idle"], "p_idle");
    } else {
      PowerDown();
      Audio::Play(sound_effects["power_down"]);
      Audio::StopLoop("p_idle");
    }

    mainPower = !mainPower;
  }

  void CycleDestination() {
    Audio::Play(sound_effects["low_beep"]);
    currentDestination += 1;
    if (currentDestination >= 4) {
      currentDestination = 1;
    }
  }

  void EnterShip() {
    if (thrusters) {
      // Audio::PlayLoop(sound_effects["thrusters_idle"], "t_idle");
    }
    if (mainPower) {
      Audio::PlayLoop(sound_effects["powered_idle"], "p_idle");
    }

    Audio::StopLoop("mining_sounds");
    Audio::StopLoop("gas_ambient");

    Audio::PlayLoop(space_music[Math::RandInt(0, 1)], "bg_music");

    onShip = true;
  }

  void ExitShip() {
    onShip = false;
    Audio::StopLoop("p_idle");
    Audio::StopLoop("t_idle");
    Audio::StopLoop("bg_music");

    switch (currentLocation) {
    case 1:
      Audio::PlayLoop("dat/music/gas_music_low.mp3", "gas_ambient");
      break;
    case 2:
      Audio::PlayLoop("dat/sfxs/mining_bg.wav", "mining_sounds");
      break;
    case 3:
      break;
    }
  }

  void ToggleThrusters() {

    if (curState == ShipState::In_Transit) {
      TerminalThrowError("CURRENTLY IN TRANSIT");
      return;
    }
    // Get them sound effects going BOO YA!
    if (!thrusters) {
      Audio::Play(sound_effects["thrusters_start"]);
      // Audio::PlayLoop(sound_effects["thrusters_idle"], "t_idle");
    } else {
      // Audio::StopLoop("t_idle");
      Audio::Play(sound_effects["thrusters_shutdown"]);
    }

    thrusters = !thrusters;
  }

  void ResetValues() {
    money = 2500;
    fuel = 80;
    fuelCapacity = 80;
    shipHealth = 100;
    cargoCapacity = 750;

    cargo.clear();

    currentLocation = 0;
  }

  float GetCargoWeight() const {
    float weight = 0.f;
    for (const CargoStack &stack : cargo) {
      weight += stack.item.weight * stack.quantity;
    }

    // factor in fuel weight (5lbs per liter to test)
    weight += fuel * 1.5f;

    return weight;
  }

  void AddCargo(const SellableItem &item, int quantity) {

    // check if we already have it, and stack it
    for (CargoStack &stack : cargo) {
      if (stack.item.itemID == item.itemID) {
        stack.quantity += quantity;
        return;
      }
    }

    // otherwise, add it
    cargo.push_back({item, quantity});
  }

  bool RemoveCargo(int itemId, int quantity) {
    for (auto it = cargo.begin(); it != cargo.end(); ++it) {
      if (it->item.itemID == itemId) {
        if (it->quantity < quantity)
          return false;

        it->quantity -= quantity;

        if (it->quantity <= 0)
          cargo.erase(it);

        return true;
      }
    }

    return false;
  }

  bool BuyCart(const std::vector<CartItem> &cart) {
    float totalCost = 0.f;
    float totalWeight = 0.f;

    // Calculate totals first
    for (const CartItem &cartItem : cart) {
      totalCost += cartItem.item.buyPrice * cartItem.quantity;
      totalWeight += cartItem.item.item.weight * cartItem.quantity;
    }

    // Check money
    if (totalCost > money) {
      TerminalThrowError("INSUFFICIENT CREDITS IN ACCOUNT");
      return false;
    }

    // Check cargo capacity
    if (GetCargoWeight() + totalWeight > cargoCapacity) {
      TerminalThrowError("NO SPACE IN THE CARGO HOLD");
      return false;
    }

    // Everything is valid, so commit the purchase
    money -= totalCost;

    for (const CartItem &cartItem : cart) {
      AddCargo(cartItem.item.item, cartItem.quantity);
    }

    return true;
  }

  bool Refuel(float amount, float pricePerLiter) {
    if (amount <= 0.f)
      return false;

    float availableSpace = fuelCapacity - fuel;

    if (amount > availableSpace)
      amount = availableSpace;

    float cost = amount * pricePerLiter;

    if (cost > money)
      return false;

    money -= cost;
    fuel += amount;

    return true;
  }

  void CheckState() {
    switch (curState) {
    case ShipState::In_Space:
      break;
    case ShipState::In_Transit:
      if (arrivalTime <= 0.f) {
        curState = In_Space;
        Audio::StopLoop("in_flight");
        Audio::Play(sound_effects["end_flight"]);
        ArriveAtDestination();
      }
      break;
    case ShipState::Hovering:
      break;
    case ShipState::Landed:
      break;
    }
  }

  bool Launch() {
    if (!mainPower) {
      return false;
    }

    if (curState == ShipState::In_Transit) {
      TerminalThrowError("CURRENTLY IN TRANSIT");
      return false;
    }

    if (!thrusters) {
      TerminalThrowError("ACTIVATE THRUSTERS TO LAUNCH");
      return false;
    }

    // just for testing
    const float fuelCost = 10.f;

    if (fuel < fuelCost) {
      TerminalThrowError("INSUFFICIENT FUEL TO LAUNCH");
      return false;
    }

    if (currentLocation == currentDestination) {
      TerminalThrowError("ALREADY AT DESTINATION");
      return false;
    }

    fuel -= fuelCost;

    currentLocation = 0;
    arrivalTime = 10.f;
    curState = ShipState::In_Transit;
    Audio::PlayLoop(sound_effects["thrusters_idle"], "in_flight");
    Utilities::Lerp("flight_time", &arrivalTime, 0.f, 10.f);

    return true;
  }

  void TerminalThrowError(std::string message) {
    // Set the error message
    shipErrorMessage = message;
    showShipErrorMessage = true;
    errorMessageLength = 0;

    // Lerp the error showing and the letters shown
    Utilities::SetVarInSeconds("terminal_error", &showShipErrorMessage, 6.f);
    Utilities::Lerp("terminal_message_length", &errorMessageLength,
                    shipErrorMessage.size() - 1, 1.f);

    // play the audio
    Audio::Play(sound_effects["terminal_error"]);
    Audio::PlayLoop(sound_effects["terminal_type"], "terminal_typing");
  }

  std::string GetTerminalError() {
    int errorMessageEndNum = std::floor(errorMessageLength);

    if (errorMessageLength >= shipErrorMessage.size() - 1) {
      Audio::StopLoop("terminal_typing");
      return shipErrorMessage;
    }

    return shipErrorMessage.substr(0, errorMessageEndNum);
  }

  void ArriveAtDestination() {
    currentLocation = currentDestination;
    switch (currentLocation) {
    case 1:
      currentMap = gasMap;
      break;
    case 2:
      currentMap = minerCavesMap;
      break;
    case 3:
      currentMap = bazaarMap;
      break;
    }
  }
};

static ImVec4 GetTileColor(int tile) {
  switch (tile) {
  case WALL:
    return ImVec4(0.5f, 0.5f, 0.55f, 1.0f); // steel grey
  case GROUND:
    return ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // dim floor
  case SHELF:
    return ImVec4(0.75f, 0.55f, 0.2f, 1.0f); // amber, matches stall crates
  case SHIP_STAIRS:
    return ImVec4(0.3f, 0.7f, 1.0f, 1.0f); // blue, stands out as an exit
  case TRADER_NPC:
    return ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // red, matches your sketch
  case RANDOM_NPC:
    return ImVec4(0.3f, 0.4f, 1.0f, 1.0f); // blue, matches your sketch
  case CHECKOUT_TERMINAL:
    return ImVec4(0.3f, 1.0f, 0.4f, 1.0f); // green, draws the eye
  default:
    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // AIR / unknown - plain white
  }
}

class LevelEditor {
  static const int GRID_SIZE = 20;
  int grid[GRID_SIZE][GRID_SIZE] = {}; // all zero-init = AIR
  int selectedTile = WALL;

public:
  bool visible = false;

  void Draw() {
    if (!visible)
      return;

    ImGui::Begin("LEVEL EDITOR", &visible);

    // =========================================================
    // TILE PALETTE
    // =========================================================
    ImGui::Text("PALETTE (click to select)");
    ImGui::Separator();

    DrawPaletteButton("AIR", AIR);
    ImGui::SameLine();
    DrawPaletteButton("GROUND", GROUND);
    ImGui::SameLine();
    DrawPaletteButton("WALL", WALL);
    ImGui::SameLine();
    DrawPaletteButton("STAIRS", SHIP_STAIRS);

    DrawPaletteButton("SHELF", SHELF);
    ImGui::SameLine();
    DrawPaletteButton("TRADER", TRADER_NPC);
    ImGui::SameLine();
    DrawPaletteButton("NPC", RANDOM_NPC);
    ImGui::SameLine();
    DrawPaletteButton("TERM", CHECKOUT_TERMINAL);

    ImGui::Spacing();
    ImGui::Text("Selected: %c  |  Left-click/drag = paint, Right-click = erase",
                GetTileCharacter(selectedTile));
    ImGui::Separator();
    ImGui::Spacing();

    // =========================================================
    // GRID
    // =========================================================
    ImGui::PushFont(Engine::Instance().getFont("ui"));

    for (int y = 0; y < GRID_SIZE; y++) {
      for (int x = 0; x < GRID_SIZE; x++) {
        int tile = grid[y][x];
        std::string label = std::string(1, GetTileCharacter(tile)) + "##cell" +
                            std::to_string(y) + "_" + std::to_string(x);

        ImGui::PushStyleColor(ImGuiCol_Text, GetTileColor(tile));
        ImGui::Button(label.c_str(), ImVec2(20, 20));
        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered()) {
          if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            grid[y][x] = selectedTile;
          } else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            grid[y][x] = AIR;
          }
        }

        if (x < GRID_SIZE - 1)
          ImGui::SameLine();
      }
    }

    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Separator();

    // =========================================================
    // EXPORT / CLEAR
    // =========================================================
    if (ImGui::Button("EXPORT TO CLIPBOARD", ImVec2(200, 30))) {
      std::string exported = ExportGrid();
      ImGui::SetClipboardText(exported.c_str());
      ConsoleLog(exported, text::green);
    }

    ImGui::SameLine();

    if (ImGui::Button("CLEAR", ImVec2(100, 30))) {
      for (int y = 0; y < GRID_SIZE; y++)
        for (int x = 0; x < GRID_SIZE; x++)
          grid[y][x] = AIR;
    }

    ImGui::End();
  }

private:
  void DrawPaletteButton(const char *label, int tileID) {
    bool selected = (selectedTile == tileID);

    if (selected)
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));

    if (ImGui::Button(label, ImVec2(70, 25))) {
      selectedTile = tileID;
    }

    if (selected)
      ImGui::PopStyleColor();
  }

  std::string ExportGrid() {
    std::string out = "std::vector<std::vector<int>> customMap = {\n";

    for (int y = 0; y < GRID_SIZE; y++) {
      out += "    {";
      for (int x = 0; x < GRID_SIZE; x++) {
        out += std::to_string(grid[y][x]);
        if (x < GRID_SIZE - 1)
          out += ", ";
      }
      out += "}";
      if (y < GRID_SIZE - 1)
        out += ",";
      out += "\n";
    }

    out += "};";
    return out;
  }
};
