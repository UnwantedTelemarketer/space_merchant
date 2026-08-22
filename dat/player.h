#include "antibox/core/antibox.h"
#include "entities.h"
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

  std::map<std::string, std::string> sound_effects = {
      {"thrusters_start", "dat/sfxs/engine_startup.wav"},
      {"thrusters_idle", "dat/sfxs/engines_idle.wav"},
      {"thrusters_shutdown", "dat/sfxs/engine_shutdown.wav"},
      {"powered_idle", "dat/sfxs/ambient_on.mp3"},
      {"power_up", "dat/sfxs/startup_ship.wav"},
      {"power_down", "dat/sfxs/power_down.wav"},
      {"terminal_error", "dat/sfxs/error.mp3"},
      {"terminal_type", "dat/sfxs/terminal_type.mp3"},
      {"sam_power_on", "dat/sfxs/sam/sam_power_on.wav"},
      {"sam_thrusters_on", "dat/sfxs/sam/sam_thrusters_on.wav"},
      {"sam_thrusters_off", "dat/sfxs/sam/sam_thrusters_off.wav"},
      {"sam_incoming_comms", "dat/sfxs/sam/sam_incoming_comms.wav"},
      {"flight_sound", "dat/sfxs/in_flight.wav"},
      {"end_flight", "dat/sfxs/rumbling_end.wav"}};

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

  void EnterShip() {
    if (thrusters) {
      // Audio::PlayLoop(sound_effects["thrusters_idle"], "t_idle");
    }
    if (mainPower) {
      Audio::PlayLoop(sound_effects["powered_idle"], "p_idle");
    }

    onShip = true;
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
        currentLocation = currentDestination;
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

  void ArriveAtDestination() { currentLocation = currentDestination; }
};
