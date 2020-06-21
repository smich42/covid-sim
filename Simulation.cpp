#include "Simulation.h"
#include <iostream>
#include <time.h>
#include <stdlib.h>

bool occurs(double probability)
{
    return ((double) rand() / (double) RAND_MAX) < probability;
};

int curInfected = INITIAL_CASES;
int maxInfected = INITIAL_CASES;
int totalInfected = INITIAL_CASES;
int occupiedBeds = 0;

std::vector<std::shared_ptr<Location>> locations;
std::vector<std::shared_ptr<Person>> people;

void Location::addPerson(std::shared_ptr<Person> p)
{
    visitors.push_back(p);
}

void Location::processDay(Distancing distancing)
{
    int infectedCnt = 0;

    for (auto p : visitors)
    {
        if (p->curStatus == Status::DIAGNOSED || p->curStatus == Status::ASYMPTOMATIC)
        {
            ++infectedCnt;
        }
    }

    double infectionProbability = 1.0 - std::pow(1.0 - INFECTION_RATES[distancing], infectedCnt);

    for (auto p : visitors)
    {
        if (p->curStatus == Status::HEALTHY && occurs(infectionProbability))
        {
            p->curStatus = Status::ASYMPTOMATIC;
            ++curInfected;
            ++totalInfected;
        }
    }

    visitors.clear();
}

int Person::count = 0;

Person::Person()
{
    int rnd = rand() % 100;

    age = Ages::ELDERLY;

    int minorThreshold = POPULATION_PER_100[Ages::MINOR];
    int youngThreshold = minorThreshold + POPULATION_PER_100[Ages::YOUNG];
    int olderThreshold = youngThreshold + POPULATION_PER_100[Ages::OLDER];

    if (rnd < olderThreshold)
    {
        age = Ages::OLDER;
    }
    if (rnd < youngThreshold)
    {
        age = Ages::YOUNG;
    }
    if (rnd < minorThreshold)
    {
        age = Ages::MINOR;
    }

    daysInfected = 0;

    if (count < INITIAL_CASES)
    {
        curStatus = Status::ASYMPTOMATIC;
    }
    else
    {
        curStatus = Status::HEALTHY;
    }

    id = count++;
};

void Person::addDay()
{
    if (curStatus == Status::DEAD) return;

    if (curStatus == Status::ASYMPTOMATIC)
    {
        ++daysInfected;

        if (daysInfected > ASYMPTOMATIC_DAYS)
        {
            curStatus = Status::DIAGNOSED;

            ++occupiedBeds;
        }
    }

    else if (curStatus == Status::DIAGNOSED)
    {
        ++daysInfected;

        if (daysInfected == INFECTION_DAYS)
        {
            double fatalityProbability = FATALITY_RATES[age];

            if (occupiedBeds > BED_COUNT)
            {
                fatalityProbability *= BED_ABSENCE_FATALITY_MODIFIER;
            }

            else if (occupiedBeds > BED_COUNT * BED_OVERWORKED_THRESHOLD_REL)
            {
                fatalityProbability *= BED_OVERWORKED_FATALITY_MODIFIER;
            }

            curStatus = occurs(fatalityProbability) ? Status::DEAD : Status::IMMUNE;

            daysInfected = 0;

            --occupiedBeds;
            --curInfected;
        }

        return;
    }

    if (maxInfected >= QUARANTINE_THRESHOLD)
    {
        if (occurs(CHANCE_OF_VISIT_WITH_QUARANTINE))
        {
            locations[rand() % LOCATION_COUNT]->addPerson(people[id]);
        }
    }
    else if (occurs(CHANCE_OF_VISIT_NO_QUARANTINE))
    {
        locations[rand() % LOCATION_COUNT]->addPerson(people[id]);
    }
};

struct SimulationData
{
    int healthy;
    int dead;
    int totalCases;
    int peakCases;
    int days;
};

SimulationData driveSimulation(Distancing distancing)
{
    curInfected = INITIAL_CASES;
    maxInfected = INITIAL_CASES;
    totalInfected = INITIAL_CASES;
    occupiedBeds = 0;

    Person::count = 0;

    srand(time(0));

    people.clear();
    locations.clear();

    for (int i = 0; i < std::max(POPULATION, LOCATION_COUNT); ++i)
    {
        if (i < POPULATION)
        {
            people.push_back(std::make_shared<Person>(Person()));
        }

        if (i < LOCATION_COUNT)
        {
            locations.push_back(std::make_shared<Location>(Location()));
        }
    }

    int days = 0;

    while (curInfected > 0)
    {
        std::cout << "Day " << days + 1;

        for (auto p : people)
        {
            p->addDay();
        }

        for (auto l : locations)
        {
            l->processDay(distancing);
        }

        std::cout << '\r';

        if (curInfected > maxInfected)
        {
            maxInfected = curInfected;
        }

        ++days;
    }

    int healthy = 0;
    int dead = 0;

    for (auto p : people)
    {
        if (p->curStatus == Status::HEALTHY || p->curStatus == Status::IMMUNE)
        {
            ++healthy;
        }
        else if (p->curStatus == Status::DEAD)
        {
            ++dead;
        }
    }

    return { healthy, dead, totalInfected, maxInfected, days };
}

void printData(SimulationData data)
{
    std::cout.precision(2);

    double percentDead = (double) data.dead / (double) POPULATION * 100.0;
    double percentBeds = (double) data.peakCases / (double) BED_COUNT * 100.0;
    double percentInfected = (double) data.totalCases / (double) POPULATION * 100.0;

    std::cout << "Days: " << data.days << '\n' << std::endl;

    std::cout << "Total cases: " << data.totalCases << " (" << std::fixed << percentInfected << "%)" << std::endl;
    std::cout << "Peak cases: " << data.peakCases << " (" << std::fixed << percentBeds << "% of beds)" << std::endl;

    std::cout << "Healthy: " << data.healthy << std::endl;
    std::cout << "Dead: " << data.dead << " (" << percentDead << "%)" << '\n' << std::endl;
}

int main()
{
    std::cout << "-----------" << std::endl;
    std::cout << "NO DISTANCING" << std::endl;
    std::cout << "-----------" << std::endl;

    printData(driveSimulation(Distancing::NONE));

    std::cout << "-----------" << std::endl;
    std::cout << "1 METRE DISTANCING" << std::endl;
    std::cout << "-----------" << std::endl;
    printData(driveSimulation(Distancing::SINGLE));

    std::cout << "-----------" << std::endl;
    std::cout << "2 METRE DISTANCING" << std::endl;
    std::cout << "-----------" << std::endl;
    printData(driveSimulation(Distancing::DOUBLE));

    return 0;
}
