#include "dat/player.h"

using namespace antibox;

#define VGAFONT "./dat/fonts/VGA437.ttf"

class Merchant : public App {
  WindowProperties GetWindowProperties() {
    WindowProperties props;

    props.imguiProps = {true, true, false, {VGAFONT}, {"ui"}, 16.f};
    props.w = 1280;
    props.h = 720;
    props.title = "Space Merchant";
    props.cc = {0.2f, 0.2f, 0.2f, 1.f};
    props.vsync = 1;
    return props;
  }

  LevelEditor levelEditor;

  // =========================================================
  // CORE STATE
  // =========================================================

  Scene main = {"BEST"};
  int int_value = 0;
  PlayerShip mainShip;
  std::vector<CartItem> shoppingCart;
  bool inTerminal = false;
  bool showInventoryLog = false;

  int playerX = 4;
  int playerY = 5;

  float sfx_volume = 0.65f;
  float music_volume = 0.4f;

  std::string playerName = "Test Player";
  std::string playerTitle = "'The Traveler'";
  SellableItem item_water = {1, "Water", 5.f, 2.f};

  std::string terminalCommand = "";
  std::string lastTerminalOutput =
      "> TERMINAL BOOTED\n> TYPE HELP FOR COMMANDS";

  // =========================================================
  // AUDIO
  // =========================================================

  std::map<std::string, std::string> sound_effects = {
      {"switch", "dat/sfxs/switch.wav"},
  };
  std::vector<std::string> kb_sounds = {
      "dat/sfxs/kb/keyboard_1.wav",
      "dat/sfxs/kb/keyboard_2.wav",
      "dat/sfxs/kb/keyboard_3.wav",
      "dat/sfxs/kb/keyboard_4.wav",
  };

  // =========================================================
  // SHIP CONTROL COLORS
  // =========================================================

  ImVec4 windowBG = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
  ImVec4 childBG = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
  ImVec4 frameBG = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);
  ImVec4 buttonColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
  ImVec4 buttonHoverColor = ImVec4(0.19f, 0.19f, 0.19f, 1.0f);
  ImVec4 buttonActiveColor = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);

  // =========================================================
  // ON-FOOT MAP
  // =========================================================

  // =========================================================
  // LOCATIONS & MARKETS
  // =========================================================

  // =========================================================
  // LIFECYCLE
  // =========================================================

  void Init() override {
    Engine::Instance().AddScene(&main);
    mainShip.ResetValues();

    mainShip.AddCargo(item_water, 15);

    Audio::SetVolume(sfx_volume);

    ConsoleLog(Audio::GetVolume(), text::red);

    Audio::PlayLoop(space_music[Math::RandInt(0, 1)], "bg_music");

    Audio::SetVolumeLoop(music_volume, "bg_music");
  }

  void Update() override {

    mainShip.CheckState();

    UpdateTerminalInput();

    if (Input::KeyDown(KEY_GRAVE_ACCENT)) {
      levelEditor.visible = !levelEditor.visible;
    }

    if (!mainShip.onShip) {
      int newX = playerX;
      int newY = playerY;

      if (Input::KeyDown(KEY_LEFT)) {
        newX -= 1;
      }
      if (Input::KeyDown(KEY_RIGHT)) {
        newX += 1;
      }
      if (Input::KeyDown(KEY_UP)) {
        newY -= 1;
      }
      if (Input::KeyDown(KEY_DOWN)) {
        newY += 1;
      }

      int mapHeight = static_cast<int>(mainShip.currentMap.size());
      int mapWidth =
          mapHeight > 0 ? static_cast<int>(mainShip.currentMap[0].size()) : 0;

      newX = std::clamp(newX, 0, mapWidth - 1);
      newY = std::clamp(newY, 0, mapHeight - 1);

      int destTile = GetMapTile(mainShip.currentMap, newX, newY);
      bool blocked = (destTile == WALL || destTile == SHELF);

      if (!blocked) {
        playerX = newX;
        playerY = newY;
      }

      int currentTile = GetMapTile(mainShip.currentMap, playerX, playerY);

      if (currentTile == CHECKOUT_TERMINAL || currentTile == TRADER_NPC) {
        inTerminal = true;
      } else {
        inTerminal = false;
      }

      if (currentTile == SHIP_STAIRS) {
        mainShip.EnterShip();
      }
    }
  }

  void Render() override {}

  void ImguiRender() override {
    DrawCockpitWindow();
    DrawShipControls();
    DrawTerminal();

    levelEditor.Draw();

    if (!mainShip.onShip && inTerminal) {
      LocationType curType = locations[mainShip.currentLocation].type;
      if (curType == LocationType::GasStation) {
        ShowGasNMartMarket();
      } else if (curType == LocationType::Colony ||
                 curType == LocationType::Market) {
        ShowTradingPost(mainShip.currentLocation);
      }
    }
  }

  void Shutdown() override {}

  // =========================================================
  // CART HELPERS
  // =========================================================

  void AddToCart(const MarketItem &item, int quantity) {
    for (CartItem &cartItem : shoppingCart) {
      if (cartItem.item.item.itemID == item.item.itemID) {
        cartItem.quantity += quantity;
        return;
      }
    }

    shoppingCart.push_back({item, quantity});
  }

  float GetCartCost() const {
    float total = 0.f;

    for (const CartItem &cartItem : shoppingCart) {
      total += cartItem.item.buyPrice * cartItem.quantity;
    }

    return total;
  }

  float GetCartWeight() const {
    float total = 0.f;

    for (const CartItem &cartItem : shoppingCart) {
      total += cartItem.item.item.weight * cartItem.quantity;
    }

    return total;
  }

  // =========================================================
  // MARKET WINDOWS
  // =========================================================

  std::string ShowInventoryTerminal() {
    std::string cargoManifest = "";
    cargoManifest += "> CARGO MANIFEST";
    cargoManifest += "\n> --------------------";

    if (mainShip.cargo.empty()) {
      cargoManifest += "\n> CARGO HOLD EMPTY";
    } else {
      for (const auto &item : mainShip.cargo) {
        cargoManifest += "\n> ";
        cargoManifest += item.item.name.c_str();
        cargoManifest += " x";
        cargoManifest += std::to_string(item.quantity);
      }
    }

    cargoManifest += "\n> --------------------";

    cargoManifest += "\n> CARGO: ";
    cargoManifest += std::to_string(mainShip.GetCargoWeight());
    cargoManifest += " / ";
    cargoManifest += std::to_string(mainShip.cargoCapacity);
    cargoManifest += "lbs.";

    return cargoManifest;
  }

  void ShowGasNMartMarket() {
    ImGui::Begin("GAS N MART");

    ImGui::Text("GAS N MART SPACE STOP");
    ImGui::Separator();

    // =========================================================
    // SHOPPING CART
    // =========================================================

    ImGui::BeginChild("ShoppingCart", ImVec2(350, 0), true);

    ImGui::Text("SHOPPING");
    ImGui::Separator();

    for (int i = 0; i < static_cast<int>(locations[1].market.size()); i++) {
      MarketItem &marketItem = locations[1].market[i];

      ImGui::Text("%-16s $%.2f", marketItem.item.name.c_str(),
                  marketItem.buyPrice);

      ImGui::SameLine();

      std::string buttonName = "ADD##" + std::to_string(i);

      if (ImGui::Button(buttonName.c_str())) {
        AddToCart(marketItem, 1);
      }
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("CART");

    for (int i = 0; i < static_cast<int>(shoppingCart.size()); i++) {
      CartItem &cartItem = shoppingCart[i];

      ImGui::Text("%s x%d", cartItem.item.item.name.c_str(), cartItem.quantity);

      ImGui::SameLine();

      ImGui::Text("$%.2f", cartItem.item.buyPrice * cartItem.quantity);

      ImGui::SameLine();

      std::string removeButton = "-##cart" + std::to_string(i);

      if (ImGui::Button(removeButton.c_str())) {
        cartItem.quantity--;

        if (cartItem.quantity <= 0) {
          shoppingCart.erase(shoppingCart.begin() + i);
          i--;
        }
      }

      ImGui::SameLine();

      std::string addButton = "+##cart" + std::to_string(i);

      if (ImGui::Button(addButton.c_str())) {
        cartItem.quantity++;
      }
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("TOTAL: $%.2f", GetCartCost());

    ImGui::Text("WEIGHT: %.1f / %.1f lbs", GetCartWeight(),
                mainShip.cargoCapacity);

    ImGui::Spacing();

    if (ImGui::Button("CLEAR CART")) {
      shoppingCart.clear();
    }

    ImGui::SameLine();

    if (ImGui::Button("BUY CART")) {
      if (mainShip.BuyCart(shoppingCart)) {
        shoppingCart.clear();
      }
    }

    ImGui::EndChild();

    // =========================================================
    // FUEL
    // =========================================================

    ImGui::SameLine();

    ImGui::BeginChild("FuelPump", ImVec2(250, 0), true);

    ImGui::Text("FUEL");
    ImGui::Separator();

    ImGui::Text("FUEL: %.1f / %.1f L", mainShip.fuel, mainShip.fuelCapacity);

    ImGui::Text("PRICE: $%.2f / L", locations[1].fuelPrice);

    ImGui::Spacing();

    if (ImGui::Button("+10 L", ImVec2(-1, 30))) {
      mainShip.Refuel(10.f, locations[1].fuelPrice);
    }

    if (ImGui::Button("+25 L", ImVec2(-1, 30))) {
      mainShip.Refuel(25.f, locations[1].fuelPrice);
    }

    if (ImGui::Button("FILL TANK", ImVec2(-1, 30))) {
      float needed = mainShip.fuelCapacity - mainShip.fuel;

      mainShip.Refuel(needed, locations[1].fuelPrice);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("MONEY: $%.2f", mainShip.money);

    ImGui::EndChild();

    ImGui::End();
  }

  NPC *GetLocationNPC(int locationIndex) {
    if (locationIndex < 0 ||
        locationIndex >= static_cast<int>(locationNPCs.size()))
      return nullptr;
    if (locationNPCs[locationIndex].name.empty())
      return nullptr;
    return &locationNPCs[locationIndex];
  }

  void ShowTradingPost(int locationIndex) {
    Location &loc = locations[locationIndex];
    NPC *npc = GetLocationNPC(locationIndex);

    ImGui::Begin(loc.name.c_str());

    // =========================================================
    // NPC / DIALOGUE
    // =========================================================

    if (npc) {
      static std::map<int, int> dialogueIndex;
      int &lineIdx = dialogueIndex[locationIndex];

      ImGui::BeginChild("NPCPanel", ImVec2(0, 90), true);
      ImGui::Text("%s", npc->name.c_str());
      ImGui::SameLine();
      ImGui::TextDisabled("- %s", npc->title.c_str());
      ImGui::Separator();

      if (!npc->dialogue.empty()) {
        ImGui::TextWrapped(
            "\"%s\"", npc->dialogue[lineIdx % npc->dialogue.size()].c_str());
      }

      if (ImGui::Button("TALK")) {
        lineIdx++;
      }
      ImGui::EndChild();
      ImGui::Spacing();
    }

    // =========================================================
    // BUY (their goods -> your cargo)
    // =========================================================

    ImGui::BeginChild("BuyPanel", ImVec2(400, 0), true);
    ImGui::Text("FOR SALE");
    ImGui::Separator();

    for (int i = 0; i < static_cast<int>(loc.market.size()); i++) {
      MarketItem &marketItem = loc.market[i];
      if (marketItem.buyPrice <= 0.f)
        continue; // not offered here

      ImGui::Text("%-16s $%.2f", marketItem.item.name.c_str(),
                  marketItem.buyPrice);
      ImGui::SameLine();

      std::string buttonName = "BUY##buy" + std::to_string(i);
      if (ImGui::Button(buttonName.c_str())) {
        if (marketItem.buyPrice > mainShip.money) {
          mainShip.TerminalThrowError("INSUFFICIENT CREDITS IN ACCOUNT");
        } else if (mainShip.GetCargoWeight() + marketItem.item.weight >
                   mainShip.cargoCapacity) {
          mainShip.TerminalThrowError("NO SPACE IN THE CARGO HOLD");
        } else {
          mainShip.money -= marketItem.buyPrice;
          mainShip.AddCargo(marketItem.item, 1);
        }
      }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // =========================================================
    // SELL (your cargo -> their credits)
    // =========================================================

    ImGui::BeginChild("SellPanel", ImVec2(400, 0), true);
    ImGui::Text("SELL CARGO");
    ImGui::Separator();

    for (int i = 0; i < static_cast<int>(mainShip.cargo.size()); i++) {
      CargoStack &stack = mainShip.cargo[i];

      // find matching market entry to see if/what they'll pay
      float sellPrice = 0.f;
      for (const MarketItem &m : loc.market) {
        if (m.item.itemID == stack.item.itemID) {
          sellPrice = m.sellPrice;
          break;
        }
      }

      ImGui::Text("%-16s x%d", stack.item.name.c_str(), stack.quantity);
      ImGui::SameLine();

      if (sellPrice > 0.f) {
        ImGui::Text("$%.2f", sellPrice);
        ImGui::SameLine();

        std::string sellButton = "SELL##sell" + std::to_string(i);
        if (ImGui::Button(sellButton.c_str())) {
          mainShip.money += sellPrice;
          mainShip.RemoveCargo(stack.item.itemID, 1);
          i--; // list may have shrunk, recheck this index
        }
      } else {
        ImGui::TextDisabled("NOT WANTED HERE");
      }
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Text("MONEY: $%.2f", mainShip.money);

    ImGui::End();
  }

  // =========================================================
  // COCKPIT WINDOW
  // =========================================================

  void DrawStarfield(ImVec2 windowPos, ImVec2 windowSize,
                     ImDrawList *drawList) {
    struct Star {
      ImVec2 position;
      bool large;
    };
    static bool initialized = false;
    static std::vector<Star> stars;

    if (!initialized) {
      initialized = true;
      std::mt19937 rng(12345);
      std::uniform_real_distribution<float> xDist(0.0f, windowSize.x);
      std::uniform_real_distribution<float> yDist(0.0f, windowSize.y);
      std::uniform_int_distribution<int> sizeDist(0, 4);
      for (int i = 0; i < 150; i++) {
        stars.push_back({ImVec2(xDist(rng), yDist(rng)), sizeDist(rng) == 0});
      }
    }

    for (const Star &star : stars) {
      ImVec2 pos(windowPos.x + star.position.x, windowPos.y + star.position.y);
      if (star.large) {
        drawList->AddText(pos, IM_COL32(220, 220, 220, 255), "*");
      } else {
        drawList->AddText(pos, IM_COL32(130, 130, 130, 255), ".");
      }
    }
  }

  void DrawHyperspeedWindow(ImVec2 windowPos, ImVec2 windowSize,
                            ImDrawList *drawList) {
    float cellWidth = ImGui::CalcTextSize("M").x;
    float cellHeight = ImGui::GetTextLineHeight();

    int cols = (int)(windowSize.x / cellWidth);
    int rows = (int)(windowSize.y / cellHeight);
    int centerCol = cols / 2;
    int centerRow = rows / 2;

    struct Streak {
      float dirX, dirY; // unit vector, one of 12 fixed angles
      float distSteps;  // current distance from center, in grid cells
                        // (float for smooth angles)
      float speedTier;  // cells advanced per tick
      bool large;
    };
    static bool initialized = false;
    static std::vector<Streak> streaks;
    static float tickTimer = 0.0f;
    const float tickInterval = 0.045f;

    if (!initialized) {
      initialized = true;
      std::mt19937 rng(12345);
      std::uniform_int_distribution<int> angleIdxDist(
          0, 11); // 12 angles, 30 degrees apart
      std::uniform_int_distribution<int> startDist(0, 20);
      std::uniform_real_distribution<float> speedDist(1.0f, 3.0f);
      std::uniform_int_distribution<int> sizeDist(0, 4);

      int maxRadius = std::max(cols, rows) / 2 + 2;
      for (int i = 0; i < 120; i++) {
        int angleIdx = angleIdxDist(rng);
        float angle = angleIdx * (3.14159f / 6.0f); // 30 degrees per step
        float dx = std::cos(angle);
        float dy = std::sin(angle);
        streaks.push_back({dx, dy, (float)(startDist(rng) % maxRadius),
                           speedDist(rng), sizeDist(rng) == 0});
      }
    }

    tickTimer += ImGui::GetIO().DeltaTime;
    bool tick = false;
    if (tickTimer >= tickInterval) {
      tickTimer -= tickInterval;
      tick = true;
    }

    int maxRadius = std::max(cols, rows) / 2 + 2;

    for (Streak &s : streaks) {
      if (tick) {
        s.distSteps += s.speedTier;
        if (s.distSteps > maxRadius) {
          s.distSteps = 0.0f;
        }
      }

      // Snap glyph choice to nearest of the 4 line characters available in
      // ASCII, based on the actual (finer-grained) direction angle
      float angle = std::atan2(s.dirY, s.dirX);
      float normAngle = std::fmod(angle + 2.0f * 3.14159f, 3.14159f); // 0..pi
      const char *glyph;
      if (normAngle < 0.3927f || normAngle > 2.7489f)
        glyph = "-";
      else if (normAngle < 1.1781f)
        glyph = "\\";
      else if (normAngle < 1.9635f)
        glyph = "|";
      else
        glyph = "/";

      int trailCount = s.large ? 4 : 2;
      for (int t = 0; t < trailCount; t++) {
        float d = s.distSteps - t;
        if (d <= 0.0f)
          continue;

        // Position is computed in float space along the true angle,
        // then rounded to the nearest grid cell so it still snaps visually.
        int cellX = centerCol + (int)std::round(s.dirX * d);
        int cellY = centerRow + (int)std::round(s.dirY * d);
        if (cellX < 0 || cellX >= cols || cellY < 0 || cellY >= rows)
          continue;

        ImVec2 pos(windowPos.x + cellX * cellWidth,
                   windowPos.y + cellY * cellHeight);

        float fade = 1.0f - (float)t / trailCount;
        int alpha = (int)(255.0f * std::min(1.0f, d / 6.0f) * fade);
        if (alpha <= 0)
          continue;

        ImU32 color = s.large ? IM_COL32(255, 255, 255, alpha)
                              : IM_COL32(160, 160, 200, alpha);

        drawList->AddText(pos, color, glyph);
      }
    }
  }

  void DrawCockpitWindow() {
    ImGui::Begin("COCKPIT WINDOW");

    if (!mainShip.onShip) {
      ImGui::PushFont(Engine::Instance().getFont("ui"));
      for (int y = 0; y < mainShip.currentMap.size(); y++) {
        for (int x = 0; x < mainShip.currentMap[0].size(); x++) {
          if (y == playerY && x == playerX) {
            ImGui::TextUnformatted("@");
          } else {
            int tile = GetMapTile(mainShip.currentMap, x, y);
            char tileCharacter = GetTileCharacter(tile);
            ImGui::TextColored(GetTileColor(tile), "%c", tileCharacter);
          }
          ImGui::SameLine();
        }
        ImGui::Spacing();
      }
      ImGui::PopFont();
      ImGui::End();
      return;
    }
    ImVec2 windowPos = ImGui::GetCursorScreenPos();
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImDrawList *drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
        windowPos,
        ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
        IM_COL32(5, 5, 10, 255));

    if (mainShip.curState == ShipState::In_Transit) {
      DrawHyperspeedWindow(windowPos, windowSize, drawList);
    } else {
      DrawStarfield(windowPos, windowSize, drawList);
    }

    ImGui::Dummy(windowSize);
    ImGui::End();
  }

  // =========================================================
  // SHIP CONTROL WINDOW
  // =========================================================

  void DrawShipControls() {
    ImGui::PushFont(Engine::Instance().getFont("ui"));
    static float throttle = 0.0f;
    static float maneuvering = 0.0f;

    ImVec4 mod = mainShip.cabinLights ? ImVec4(0.25f, 0.25f, 0.25f, 1.f)
                                      : ImVec4(0.f, 0.f, 0.f, 1.f);

    ImGui::PushStyleColor(ImGuiCol_WindowBg,
                          (Utilities::AddVectors(windowBG, mod)));
    ImGui::PushStyleColor(ImGuiCol_ChildBg,
                          (Utilities::AddVectors(childBG, mod)));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,
                          (Utilities::AddVectors(frameBG, mod)));
    ImGui::PushStyleColor(ImGuiCol_Button,
                          (Utilities::AddVectors(buttonColor, mod)));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          (Utilities::AddVectors(buttonHoverColor, mod)));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          (Utilities::AddVectors(buttonActiveColor, mod)));

    ImGui::Begin("SHIP CONTROL");

    if (mainShip.onShip == false) {
      ImGui::Text("NOT ON SHIP");
      ImGui::PopFont();
      ImGui::End();
      ImGui::PopStyleColor(6);

      return;
    }

    ImGui::Text("THE WAYFARER");
    ImGui::Separator();
    ImGui::Spacing();

    // Calculate width for 3 evenly-sized panels
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float panelWidth = (availableWidth - spacing * 2.0f) / 3.0f;

    // ---------------------------------------------------------
    // FLIGHT
    // ---------------------------------------------------------

    ImGui::BeginChild("Flight", ImVec2(panelWidth, 0), true);

    ImGui::Text("FLIGHT");
    ImGui::Separator();

    ImGui::Text("Throttle");

    ImGui::SliderFloat("##Throttle", &throttle, 0.0f, 100.0f, "%.0f%%");

    if (mainShip.thrusters) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.65f, 0.1f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(0.15f, 0.8f, 0.15f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                            ImVec4(0.2f, 0.9f, 0.2f, 1.0f));
    } else {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.2f, 0.05f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(0.08f, 0.3f, 0.08f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                            ImVec4(0.1f, 0.4f, 0.1f, 1.0f));
    }

    const char *thrustText =
        mainShip.thrusters ? "THRUSTERS ACTIVE" : "THRUSTERS OFF";

    if (ImGui::Button(thrustText, ImVec2(-1, 35)) && mainShip.mainPower) {
      mainShip.ToggleThrusters();
    }

    ImGui::PopStyleColor(3);

    ImGui::Text("CURRENT LOCATION");
    std::string curText = "   - ";
    curText += locations[mainShip.currentLocation].name;
    ImGui::Text(curText.c_str());

    ImGui::Text("DESTINATION");
    std::string destText = "   - ";
    destText += locations[mainShip.currentDestination].name;
    ImGui::Text(destText.c_str());

    if (ImGui::Button("CYCLE DESTINATION", ImVec2(-1, 35))) {
      mainShip.CycleDestination();
    }

    ImGui::Spacing();

    if (ImGui::Button("LAUNCH", ImVec2(-1, 40))) {
      mainShip.Launch();
    }

    if (mainShip.currentLocation != 0) { // anywhere but Deep Space
      if (ImGui::Button("EXIT SHIP")) {
        mainShip.ExitShip();
        playerX = 4;
        playerY = 5;
      }
    }

    ImGui::EndChild();

    ImGui::SameLine();

    // ---------------------------------------------------------
    // SHIP SYSTEMS
    // ---------------------------------------------------------

    ImGui::BeginChild("Systems", ImVec2(panelWidth, 0), true);

    ImGui::Text("SHIP SYSTEMS");
    ImGui::Separator();

    if (ImGui::Button("Landing Gear") && mainShip.mainPower) {
      mainShip.landingGear = !mainShip.landingGear;
      Audio::Play(sound_effects["switch"]);
    }

    if (ImGui::Button("Cabin Lights") && mainShip.mainPower) {
      mainShip.cabinLights = !mainShip.cabinLights;
      Audio::Play(sound_effects["switch"]);
    }

    if (ImGui::Button("Window Wipers") && mainShip.mainPower) {
      Audio::Play(sound_effects["switch"]);
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("OTHER");

    if (ImGui::Button("RADIO", ImVec2(-1, 30))) {
      // Radio
    }

    if (ImGui::Button("SCANNER", ImVec2(-1, 30))) {
      // Scanner
    }

    if (ImGui::Button("MAIN POWER SWITCH", ImVec2(-1, 30))) {
      Audio::Play(sound_effects["switch"]);
      mainShip.PowerUp();
    }

    ImGui::EndChild();

    ImGui::SameLine();

    // ---------------------------------------------------------
    // STATUS
    // ---------------------------------------------------------

    ImGui::BeginChild("Status", ImVec2(panelWidth, 0), true);

    ImGui::Text("STATUS");
    ImGui::Separator();

    ImGui::Text("HULL");
    ImGui::ProgressBar(mainShip.shipHealth / mainShip.shipHealthMax,
                       ImVec2(-1, 20), "");
    ImGui::Text("%.0f / %.0f", mainShip.shipHealth, mainShip.shipHealthMax);

    ImGui::Spacing();

    ImGui::Text("FUEL");
    std::string fuelText = std::to_string(mainShip.fuel) + "L / " +
                           std::to_string(mainShip.fuelCapacity) + "L";
    float fuelPercentage = static_cast<float>(mainShip.fuel) /
                           static_cast<float>(mainShip.fuelCapacity);
    ImGui::ProgressBar(fuelPercentage, ImVec2(-1, 20), fuelText.c_str());

    ImGui::Spacing();

    ImGui::Text("CARGO");
    std::string cargoText = std::to_string(mainShip.GetCargoWeight()) +
                            "lbs / " + std::to_string(mainShip.cargoCapacity) +
                            "lbs";
    float cargoPercentage = static_cast<float>(mainShip.GetCargoWeight()) /
                            static_cast<float>(mainShip.cargoCapacity);
    ImGui::ProgressBar(cargoPercentage, ImVec2(-1, 20), cargoText.c_str());

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("MONEY");
    ImGui::Text(std::to_string(mainShip.money).c_str());

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("INVENTORY", ImVec2(-1, 35))) {
      showInventoryLog = !showInventoryLog;
    }

    ImGui::EndChild();

    ImGui::PopFont();

    ImGui::End();

    ImGui::PopStyleColor(6);
  }

  // =========================================================
  // TERMINAL WINDOW
  // =========================================================

  void DrawTerminal() {
    // Terminal colors
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

    ImGui::Begin("TERMINAL");

    if (!mainShip.mainPower) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
      ImGui::Text("================================");
      ImGui::Text("===      MAIN POWER OFF      ===");
      ImGui::Text("================================");
      ImGui::PopStyleColor(3);
      ImGui::End();

      return; // if theres no power we dont display the terminal
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));

    if (mainShip.showShipErrorMessage) {
      TerminalErrorMessage();
    } else {
      ImGui::Text("================================");
      ImGui::Text("===       THE WAYFARER       ===");
      ImGui::Text("================================");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ShowDefaultTerminal();

    ImGui::PopStyleColor(3);
    ImGui::End();
  }

  void TerminalErrorMessage() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));

    ImGui::Text("==== !!! SYSTEM WARNING !!! ====");
    ImGui::Text("> %s", mainShip.GetTerminalError().c_str());
    ImGui::Text("======== LAUNCH ABORTED ========");

    ImGui::PopStyleColor();
  }

  std::string TerminalCheckStatus() {
    std::string statusText = "";
    std::string lGearText = "\n> LANDING GEAR .......... ";
    lGearText += mainShip.landingGear ? "ON" : "OFF";

    std::string cGearText = "\n> CABIN LIGHTS .......... ";
    cGearText += mainShip.cabinLights ? "ON" : "OFF";

    std::string wGearText = "\n> WIPERS ................ ";
    wGearText += mainShip.wipers ? "ON" : "OFF";

    statusText += "\n> SYSTEM CHECK COMPLETE";
    statusText += "\n> NAVIGATIONS ........... OK";
    statusText += "\n> LIFE SUPPORT .......... OK";
    statusText += lGearText;
    statusText += cGearText;
    statusText += wGearText;
    statusText += "\n> ALL IS GOOD";

    statusText += "\nNO ACTIVE WARNINGS";
    statusText += "\nNO SYSTEM FAULTS";
    statusText += "\nSHIP STATUS: NOMINAL";

    return statusText;
  }

  void ShowDefaultTerminal() {
    ImGui::TextWrapped("%s", lastTerminalOutput.c_str());
    ImGui::Text("--------------------------------");
    ImGui::Text("> %s", terminalCommand.c_str());
  }

  void UpdateTerminalInput() {
    if (!mainShip.onShip) {
      return; // only capture when player is at the terminal
    }

    // 65-90 is a-z on keys
    const int lowercaseOffset = 32;
    bool clicked = false;

    // Grab any characters typed this frame
    for (int curKey = KEY_A; curKey < KEY_Z + 1; curKey++) {
      if (Input::KeyDown(curKey)) { // printable ASCII only
        terminalCommand += static_cast<char>(curKey);
        clicked = true;
      } else if (Input::KeyDown(KEY_SPACE)) {
        terminalCommand += ' ';
        clicked = true;
      }
    }

    if (Input::KeyDown(KEY_BACKSPACE) && !terminalCommand.empty()) {
      terminalCommand.pop_back();
      clicked = true;
    }

    if (Input::KeyDown(KEY_ENTER)) {
      if (!terminalCommand.empty()) {
        RunTerminalCommand(terminalCommand);
      }
      terminalCommand.clear();
      clicked = true;
    }

    if (clicked) {
      Audio::Play(kb_sounds[Math::RandInt(0, 3)]);
    }
  }

  void RunTerminalCommand(std::string command) {
    std::string commandOutput = "";

    if (command == "HELP") {
      commandOutput += "> HELP\n";
      commandOutput += " - HELP         : THIS\n";
      commandOutput += " - VALUE <ITEM> : GENERAL VALUE CHECK\n";
      commandOutput += " - VALUE TOTAL  : GIVES TOTAL CARGO VALUE\n";
      commandOutput += " - CARGO        : LISTS ALL HELD CARGO\n";
      commandOutput +=
          " - LIST <PEOPLE/PLACES> : LISTS KNOWN PEOPLE OR PLACES\n";
      commandOutput += " - STATUS       : SHOWS STATUS OF SHIP\n";
      commandOutput += " - DIAG         : DIAGNOSES ERRORS\n";
      commandOutput += " - CLEAR        : CLEARS THE TERMINAL";
    } else if (command == "CARGO") {
      commandOutput = ShowInventoryTerminal();
    } else if (command == "CLEAR") {
      commandOutput = "> SCREEN CLEARED.";
    } else if (command == "STATUS") {
      commandOutput = TerminalCheckStatus();
    }

    if (!commandOutput.empty()) {
      lastTerminalOutput = commandOutput;
    }
  }

  inline int GetMapTile(const std::vector<std::vector<int>> &map, int x,
                        int y) {
    if (y < 0 || y >= static_cast<int>(map.size()))
      return AIR;
    if (x < 0 || x >= static_cast<int>(map[y].size()))
      return AIR;
    return map[y][x];
  }
};

std::vector<antibox::App *> CreateGame() { return {new Merchant}; }
