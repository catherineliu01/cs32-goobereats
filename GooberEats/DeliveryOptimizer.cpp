#include "provided.h"
#include <vector>
using namespace std;

#include <random>

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    double generateRand() const;
    int randInt(int min, int max) const;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

double DeliveryOptimizerImpl::generateRand() const
{
    static std::random_device rd;
    static std::default_random_engine generator(rd());
    std::uniform_real_distribution<double> distro(0, 1);
    return distro(generator);
}

int DeliveryOptimizerImpl::randInt(int min, int max) const
{
    if (max < min)
        std::swap(max, min);
    static std::random_device rd;
    static std::default_random_engine generator(rd());
    std::uniform_int_distribution<> distro(min, max);
    return distro(generator);
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    int count = 0;
    
    vector<DeliveryRequest> newDeliveries = deliveries;
    
    // get oldCrowDistance
    oldCrowDistance = distanceEarthMiles(depot, deliveries[0].location);
    for (int i = 0; i < deliveries.size() - 1; i++)
    {
        oldCrowDistance += distanceEarthMiles(deliveries[i].location, deliveries[i+1].location);
    }
    oldCrowDistance += distanceEarthMiles(deliveries[deliveries.size() - 1].location, depot);
    
    double temperature = 100;
    double coolingRate = 0.99; // this is kind of arbitrary... oh well
    double absoluteTemperature = 0.01;
    double currentPathCrowDistance = oldCrowDistance;
    
    while (temperature > absoluteTemperature)
    {
        vector<DeliveryRequest> tentativeNewDeliveries = newDeliveries;
        
        int rand1 = randInt(0, deliveries.size()-1);
        int rand2 = randInt(0, deliveries.size()-1);
        while (rand2 == rand1)
        {
            rand2 = randInt(0, deliveries.size()-1);
        }
        
        // reverse segment
        DeliveryRequest temp = tentativeNewDeliveries[rand1];
        tentativeNewDeliveries[rand1] = tentativeNewDeliveries[rand2];
        tentativeNewDeliveries[rand2] = temp;
        
        // get tentativeNewCrowDistance w/ reversed segments
        double tentativeNewCrowDistance = distanceEarthMiles(depot, tentativeNewDeliveries[0].location);
        for (int i = 0; i < tentativeNewDeliveries.size() - 1; i++)
        {
            tentativeNewCrowDistance += distanceEarthMiles(tentativeNewDeliveries[i].location, tentativeNewDeliveries[i+1].location);
        }
        tentativeNewCrowDistance += distanceEarthMiles(tentativeNewDeliveries[newDeliveries.size() - 1].location, depot);
        
        double costDiff = tentativeNewCrowDistance - currentPathCrowDistance;
        if (costDiff < 0)
        {
            newDeliveries = tentativeNewDeliveries;
            currentPathCrowDistance = tentativeNewCrowDistance;
        }
        else
        {
            double probability = exp((-1 * costDiff) / temperature);
            double random = generateRand();
            if (probability > random)
            {
                newDeliveries = tentativeNewDeliveries;
                currentPathCrowDistance = tentativeNewCrowDistance;
            }
        }
        
        temperature *= coolingRate;
        count++;
    }
    
    newCrowDistance = distanceEarthMiles(depot, newDeliveries[0].location);
    for (int i = 0; i < newDeliveries.size() - 1; i++)
    {
        newCrowDistance += distanceEarthMiles(newDeliveries[i].location, newDeliveries[i+1].location);
    }
    cerr << "oldCrowDistance: " << oldCrowDistance << endl;
    cerr << "newCrowDistance: " << newCrowDistance << endl;
    cerr << "count: " << count << endl;
    deliveries = newDeliveries;
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
