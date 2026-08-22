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

struct NPC {
  std::string name;
  std::string title;
  std::vector<std::string> dialogue;
};

SellableItem food{0, "Food", 20.f, 1.f};
SellableItem water{1, "Water", 10.f, 1.f};
SellableItem nugget_iron{2, "Iron Nugget", 15.f, 5.f};
SellableItem nugget_gold{3, "Gold Nugget", 25.f, 10.f};

Location gasNMart{"Gas N Mart Space Stop", LocationType::GasStation};

Location miningColony{"Mining Colony", LocationType::Colony};

Location bazaar{"The Starport Bazaar", LocationType::Market};

static std::vector<Location> locations = {
    {"Deep Space", LocationType::Space, -1.f, {}},
    {"Gas N Mart Space Stop",
     LocationType::GasStation,
     3.5f,
     {{food, 25.f, 15.f}, {water, 12.f, 7.f}}},

    {"Mining Colony",
     LocationType::Colony,
     10.f,
     // sellPrice = what they pay when you sell food/water to them
     // buyPrice  = what you pay for their ore
     {{food, 0.f, 30.f},
      {water, 0.f, 22.f},
      {nugget_iron, 18.f, 0.f},
      {nugget_gold, 30.f, 0.f}}},

    {"Bazaar",
     LocationType::Market,
     5.5f,
     // Bazaar buys your ore at a markup, sells food/water back
     {{food, 22.f, 0.f},
      {water, 11.f, 0.f},
      {nugget_iron, 0.f, 26.f},
      {nugget_gold, 0.f, 42.f}}},
};

// NPCs indexed to match `locations` above. Locations with an empty
// name are treated as having no NPC (see GetLocationNPC).
static std::vector<NPC> locationNPCs = {
    {}, // Deep Space
    {}, // Gas N Mart
    {"Grubb",
     "Foreman",
     {"You buyin' or sellin', trader?",
      "Ore's good this week. Take it off my hands.",
      "Careful with that cargo hold - we don't do refunds.",
      "Miners need food and water more than credits, some weeks."}},
    {"Yevani",
     "Bazaar Trader",
     {"Welcome to the Bazaar. Best prices this side of the belt.",
      "Ore, eh? I know a buyer. That buyer is me.",
      "Don't haggle. I've heard every trick.",
      "Fresh water always sells here - station's always thirsty."}},
};
