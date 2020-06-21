#include <map>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>

enum class Distancing
{
    NONE,
    SINGLE,
    DOUBLE
};

// From https://www.itv.com/news/2020-06-01/two-metre-distance-more-effective-than-one-at-curbing-covid-19-spread-study/
std::map<Distancing, double> INFECTION_RATES = {
    { Distancing::NONE, 16 / 100.0 }, // No distancing
    { Distancing::SINGLE,  2.6 / 100.0 }, // 1 metre
    { Distancing::DOUBLE, 1.3 / 100.0 }, // 2 metres
};

enum class Ages
{
    MINOR, // < 18
    YOUNG, // < 50
    OLDER, // < 80
    ELDERLY
};

// From https://www.cebm.net/covid-19/global-covid-19-case-fatality-rates/
std::map<Ages, double> FATALITY_RATES = {
    { Ages::MINOR, 0.1 / 100.0 },
    { Ages::YOUNG, 0.3 / 100.0 },
    { Ages::OLDER, 2.5 / 100.0 },
    { Ages::ELDERLY, 11.4 / 100.0 },
};

// From https://ec.europa.eu/eurostat/web/products-datasets/-/tps00010
std::map<Ages, double> POPULATION_PER_100 = {
    { Ages::MINOR, 20.0 },
    { Ages::YOUNG, 40.0 },
    { Ages::OLDER, 35.0 },
    { Ages::ELDERLY, 5.0 },
};

enum class Status
{
    HEALTHY,
    ASYMPTOMATIC,
    DIAGNOSED,
    IMMUNE,
    DEAD
};

#define INITIAL_CASES 40

#define POPULATION ((int) 1e5)

// From https://en.wikipedia.org/wiki/List_of_countries_by_hospital_beds
#define PEOPLE_PER_BED 160

#define PEOPLE_PER_LOCATION 10

#define BED_COUNT ((int) (POPULATION / PEOPLE_PER_BED))
#define LOCATION_COUNT ((int) (POPULATION / PEOPLE_PER_LOCATION))

#define BED_OVERWORKED_THRESHOLD_REL 0.9

#define BED_OVERWORKED_FATALITY_MODIFIER 1.2

#define BED_ABSENCE_FATALITY_MODIFIER 1.5

#define QUARANTINE_THRESHOLD 300

#define CHANCE_OF_VISIT_NO_QUARANTINE 0.9

#define CHANCE_OF_VISIT_WITH_QUARANTINE 0.2

#define INFECTION_DAYS 14

// From https://www.who.int/health-topics/coronavirus
#define ASYMPTOMATIC_DAYS 6

class Location;

class Person
{
public:
    Person();

    static int count;

    int id;

    int daysInfected;
    Status curStatus;

    Ages age;

    void addDay();
};

class Location
{
public:
    std::vector<std::shared_ptr<Person>> visitors;

    void addPerson(std::shared_ptr<Person> p);

    void processDay(Distancing distancing);
};
