#include "entities.h"
#include <vector>

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

  bool onShip = true;

  // cosmetic things - less important
  bool landingGear = false;
  bool cabinLights = false;
  bool wipers = false;

  bool showShipErrorMessage = false;
  std::string shipErrorMessage = "NO FUEL";

  std::vector<CargoStack> cargo;

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
      shipErrorMessage = "INSUFFICIENT CREDITS IN ACCOUNT";
      showShipErrorMessage = true;
      return false;
    }

    // Check cargo capacity
    if (GetCargoWeight() + totalWeight > cargoCapacity) {
      shipErrorMessage = "NOT ENOUGH FREE SPACE ON SHIP";
      showShipErrorMessage = true;
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

  bool Launch() {
    if (currentLocation == currentDestination) {
      shipErrorMessage = "ALREADY AT DESTINATION";
      showShipErrorMessage = true;
      return false;
    }

    // just for testing
    const float fuelCost = 10.f;

    if (fuel < fuelCost) {
      shipErrorMessage = "INSUFFICIENT FUEL TO LAUNCH";
      showShipErrorMessage = true;
      return false;
    }

    fuel -= fuelCost;

    currentLocation = 0;

    return true;
  }

  void ArriveAtDestination() { currentLocation = currentDestination; }
};
