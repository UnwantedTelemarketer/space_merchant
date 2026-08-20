#include "string"

struct SellableItem {
  int itemID = -1;

  std::string name;
  float baseValue = -1.f;
  float weight = -1.f;
};

struct MarketItem {
  SellableItem item;
  float buyPrice = 0.f;
  float sellPrice = 0.f;
};

struct CartItem {
  MarketItem item;
  int quantity = 0;
};

struct CargoStack {
  SellableItem item;
  int quantity = 0;
};

enum class LocationType { Market, Explore, Colony, GasStation, Space };

enum class InterruptEventType {
  Pirates,
  Kidnappers,
  Distress_Signal,
  Asteroid_Belt,
  NavSystemLost
};

struct Location {
  std::string name;
  LocationType type;

  float fuelPrice = 0.f;

  std::vector<MarketItem> market;
};

SellableItem food{0, "Food", 20.f, 1.f};
SellableItem water{1, "Water", 10.f, 1.f};
SellableItem nugget_iron{2, "Iron Nugget", 15.f, 5.f};
SellableItem nugget_gold{3, "Gold Nugget", 25.f, 10.f};

Location gasNMart{"Gas N Mart Space Stop", LocationType::GasStation};

Location miningColony{"Mining Colony", LocationType::Colony};

Location bazaar{"The Starport Bazaar", LocationType::Market};
