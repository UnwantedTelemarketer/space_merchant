
#include "dat/player.h"
#include "dat/world.h"

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

  Scene main = {"BEST"};
  int int_value = 0;
  PlayerShip mainShip;
  std::vector<CartItem> shoppingCart;
  bool inTerminal = false;
  bool showInventoryLog = false;

  std::map<std::string, std::string> sound_effects = {
      {"switch", "dat/sfxs/switch.wav"}};

  int playerX = 4;
  int playerY = 5;

  std::string playerName = "Test Player";
  std::string playerTitle = "'The Traveler'";
  SellableItem item_water = {1, "Water", 5.f, 2.f};

  //----Ship Colors----
  ImVec4 windowBG = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
  ImVec4 childBG = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
  ImVec4 frameBG = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);
  ImVec4 buttonColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
  ImVec4 buttonHoverColor = ImVec4(0.19f, 0.19f, 0.19f, 1.0f);
  ImVec4 buttonActiveColor = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);

  std::vector<std::vector<int>> currentMap = {
      {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
       3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
      {3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3},
      {3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3},
      {3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3},
      {3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 10, 2, 3},
      {3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3},
      {3, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 3},
      {3, 4, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2,  2, 2,
       2, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 10, 2, 3},
      {3, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 3},
      {3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3},
      {3, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2,  2, 2,
       2, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 10, 2, 3},
      {3, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 3},
      {3, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 2, 2, 3},
      {3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3},
      {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
       3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}};

  std::vector<Location> locations = {
      {"Deep Space", LocationType::Space, -1.f, {}},
      {"Gas N Mart Space Stop",
       LocationType::GasStation,
       3.5f,
       {{food, 25.f, 15.f}, {water, 12.f, 7.f}}},

      {"Mining Colony", LocationType::Colony, 10.f, {}},
      {"Bazaar", LocationType::Market, 5.5f, {}}};

  void Init() override {
    Engine::Instance().AddScene(&main);
    mainShip.ResetValues();

    mainShip.AddCargo(item_water, 15);

    Audio::SetVolume(0.65f);

    ConsoleLog(Audio::GetVolume(), text::red);
  }

  void Update() override {
    if (Input::KeyDown(KEY_GRAVE_ACCENT)) {
      Utilities::ToggleConsoleVisible();
    }

    Audio::SetVolumeLoop(0.65f, "t_idle");

    if (!mainShip.onShip) {
      if (Input::KeyDown(KEY_LEFT)) {
        playerX -= 1;
      }
      if (Input::KeyDown(KEY_RIGHT)) {
        playerX += 1;
      }
      if (Input::KeyDown(KEY_UP)) {
        playerY -= 1;
      }
      if (Input::KeyDown(KEY_DOWN)) {
        playerY += 1;
      }

      if (currentMap[playerY][playerX] == 10) {
        inTerminal = true;
      } else {
        inTerminal = false;
      }

      playerX = std::clamp(playerX, 0, 29);
      playerY = std::clamp(playerY, 0, 14);

      if (currentMap[playerY][playerX] == 4) {
        mainShip.EnterShip();
      }
    }
  }

  void Render() override {}

  void ImguiRender() override {
    DrawCockpitWindow();
    DrawShipControls();
    DrawTerminal();
    if (!mainShip.onShip && inTerminal) {
      ShowGasNMartMarket();
    }
  }

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

  void ShowInventoryTerminal() {
    ImGui::Text("> CARGO MANIFEST");
    ImGui::Spacing();

    ImGui::Text("> --------------------");

    if (mainShip.cargo.empty()) {
      ImGui::Text("> CARGO HOLD EMPTY");
    } else {
      for (const auto &item : mainShip.cargo) {
        ImGui::Text("> %-16s x%d", item.item.name.c_str(), item.quantity);
      }
    }

    ImGui::Spacing();

    ImGui::Text("> --------------------");

    ImGui::Text("> CARGO: %d / %.0f LBS", mainShip.GetCargoWeight(),
                mainShip.cargoCapacity);

    ImGui::Spacing();
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

  //---------------------------------------=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void DrawCockpitWindow() {
    ImGui::Begin("COCKPIT WINDOW");

    if (!mainShip.onShip) {
      ImGui::PushFont(Engine::Instance().getFont("ui"));
      for (int y = 0; y < 15; y++) {
        for (int x = 0; x < 30; x++) {
          if (y == playerY && x == playerX) {
            ImGui::TextUnformatted("@");
          } else {
            char tileCharacter = GetTileCharacter(currentMap[y][x]);
            ImGui::Text("%c", tileCharacter);
          }
          if (x < 29)
            ImGui::SameLine();
        }
      }

      ImGui::PopFont();
      ImGui::End();

      return;
    }

    ImVec2 windowPos = ImGui::GetCursorScreenPos();
    ImVec2 windowSize = ImGui::GetContentRegionAvail();

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    // Background
    drawList->AddRectFilled(
        windowPos,
        ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
        IM_COL32(5, 5, 10, 255));

    // Seed so the stars don't change every frame
    static bool initialized = false;

    struct Star {
      ImVec2 position;
      bool large;
    };

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

    // Draw stars
    for (const Star &star : stars) {
      ImVec2 pos(windowPos.x + star.position.x, windowPos.y + star.position.y);

      if (star.large) {
        drawList->AddText(pos, IM_COL32(220, 220, 220, 255), "*");
      } else {
        drawList->AddText(pos, IM_COL32(130, 130, 130, 255), ".");
      }
    }

    // Reserve the space so ImGui doesn't put widgets over it
    ImGui::Dummy(windowSize);

    ImGui::End();
  }

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

    // =========================================================
    // FLIGHT
    // =========================================================

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
      mainShip.currentDestination += 1;
      if (mainShip.currentDestination >= locations.size()) {
        mainShip.currentDestination = 1;
      }
    }

    ImGui::Spacing();

    if (ImGui::Button("LAUNCH", ImVec2(-1, 40))) {
      if (mainShip.Launch()) {
        mainShip.ArriveAtDestination();
      } else {
        Utilities::SetVarInSeconds("terminal_error",
                                   &mainShip.showShipErrorMessage, 3.f);
      }
    }
    if (mainShip.currentLocation == 1) {
      if (ImGui::Button("EXIT SHIP")) {
        mainShip.onShip = false;
        Audio::StopLoop("p_idle");
        Audio::StopLoop("t_idle");
        playerX = 4;
        playerY = 5;
      }
    }

    ImGui::EndChild();

    ImGui::SameLine();

    // =========================================================
    // SHIP SYSTEMS
    // =========================================================

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
      mainShip.PowerUp();
    }

    ImGui::EndChild();

    ImGui::SameLine();

    // =========================================================
    // STATUS
    // =========================================================

    ImGui::BeginChild("Status", ImVec2(panelWidth, 0), true);

    ImGui::Text("STATUS");
    ImGui::Separator();

    // -------------------------
    // Hull
    // -------------------------

    ImGui::Text("HULL");

    ImGui::ProgressBar(mainShip.shipHealth / mainShip.shipHealthMax,
                       ImVec2(-1, 20), "");

    ImGui::Text("%.0f / %.0f", mainShip.shipHealth, mainShip.shipHealthMax);

    ImGui::Spacing();

    // -------------------------
    // Fuel
    // -------------------------

    ImGui::Text("FUEL");

    std::string fuelText = std::to_string(mainShip.fuel) + "L / " +
                           std::to_string(mainShip.fuelCapacity) + "L";

    float fuelPercentage = static_cast<float>(mainShip.fuel) /
                           static_cast<float>(mainShip.fuelCapacity);

    ImGui::ProgressBar(fuelPercentage, ImVec2(-1, 20), fuelText.c_str());

    ImGui::Spacing();

    // -------------------------
    // Cargo
    // -------------------------

    ImGui::Text("CARGO");

    std::string cargoText = std::to_string(mainShip.GetCargoWeight()) +
                            "lbs / " + std::to_string(mainShip.cargoCapacity) +
                            "lbs";

    float cargoPercentage = static_cast<float>(mainShip.GetCargoWeight()) /
                            static_cast<float>(mainShip.cargoCapacity);

    ImGui::ProgressBar(cargoPercentage, ImVec2(-1, 20), cargoText.c_str());

    ImGui::Spacing();
    ImGui::Separator();

    // -------------------------
    // Other information
    // -------------------------

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

  void DrawTerminal() {

    // Terminal colors
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

    ImGui::Begin("TERMINAL");

    if (!mainShip.mainPower) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
      ImGui::Text("================================");
      ImGui::Text("         MAIN POWER OFF         ");
      ImGui::Text("================================");
      ImGui::PopStyleColor(3);
      ImGui::End();

      return; // if theres no power we dont display the terminal
    }
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));

    ImGui::Text("================================");
    ImGui::Text("       SHIP SYSTEM TERMINAL     ");
    ImGui::Text("================================");

    ImGui::Spacing();

    std::string lGearText = "> LANDING GEAR .......... ";
    lGearText += mainShip.landingGear ? "ON" : "OFF";

    std::string cGearText = "> CABIN LIGHTS .......... ";
    cGearText += mainShip.cabinLights ? "ON" : "OFF";

    std::string wGearText = "> WIPERS ................ ";
    wGearText += mainShip.wipers ? "ON" : "OFF";

    ImGui::Text("> BOOT SEQUENCE COMPLETE");
    ImGui::Text("> SYSTEM CHECK COMPLETE");
    ImGui::Text("> NAVIGATIONS ........... OK");
    ImGui::Text("> LIFE SUPPORT .......... OK");
    ImGui::Text(lGearText.c_str());
    ImGui::Text(cGearText.c_str());
    ImGui::Text(wGearText.c_str());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (mainShip.showShipErrorMessage) {
      TerminalErrorMessage();
    } else if (showInventoryLog) {
      ShowInventoryTerminal();
    } else {
      ShowDefaultTerminal();
    }
    ImGui::PopStyleColor(3);
    ImGui::End();
  }

  void TerminalErrorMessage() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));

    ImGui::Text("> !!! SYSTEM WARNING !!!");

    ImGui::Spacing();

    ImGui::Text("> %s", mainShip.shipErrorMessage.c_str());

    ImGui::Spacing();

    ImGui::Text("> LAUNCH ABORTED");

    ImGui::PopStyleColor();
  }

  void ShowDefaultTerminal() {

    ImGui::Text("> ALL IS GOOD");

    ImGui::Spacing();

    ImGui::Text("NO ACTIVE WARNINGS");
    ImGui::Text("NO SYSTEM FAULTS");
    ImGui::Text("SHIP STATUS: NOMINAL");

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Text("READY.");
  }

  void Shutdown() override {}
};

std::vector<antibox::App *> CreateGame() { return {new Merchant}; }
