#include "provided.h"
#include <vector>
using namespace std;


const int NO_TURN = 0;
const int LEFT_TURN = 1;
const int RIGHT_TURN = 2;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_map;
    string getDirectionForProceedCmd(double angle) const;
    int getDirectionForTurnCmd(double angle) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
    m_map = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

string DeliveryPlannerImpl::getDirectionForProceedCmd(double angle) const
{
    if (angle >= 0 && angle < 22.5) { return "east"; }
    if (angle >= 22.5 && angle < 67.5) { return "northeast"; }
    if (angle >= 67.5 && angle < 112.5) { return "north"; }
    if (angle >= 112.5 && angle < 157.5) { return "northwest"; }
    if (angle >= 157.5 && angle < 202.5) { return "west"; }
    if (angle >= 202.5 && angle < 247.5) { return "southwest"; }
    if (angle >= 247.5 && angle < 292.5) { return "south"; }
    if (angle >= 292.5 && angle < 337.5) { return "southeast"; }
    else { return "east"; }
}

int DeliveryPlannerImpl::getDirectionForTurnCmd(double angle) const
{
    if (angle < 1 || angle > 359) return NO_TURN;
    if (angle >= 1 && angle < 180) return LEFT_TURN;
    else return RIGHT_TURN;
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    DeliveryOptimizer optimize(m_map);
    vector<DeliveryRequest> newDeliveries = deliveries;
    double oldCrowDistance;
    double newCrowDistance;
    optimize.optimizeDeliveryOrder(depot, newDeliveries, oldCrowDistance, newCrowDistance);
    
    vector<StreetSegment> segments;
    if (!(m_map->getSegmentsThatStartWith(depot, segments)))
    {
        cerr << "Bad coordinate!" << endl;
        return BAD_COORD;
    }
    
    // make deliveries
    PointToPointRouter router(m_map);
    const GeoCoord* a = &depot;
    const GeoCoord* b = nullptr;
    std::list<StreetSegment> route;
    double totalDist;
    
    totalDistanceTravelled = 0;
    
    for (int i = 0; i < newDeliveries.size(); i++)
    {
        if (!(m_map->getSegmentsThatStartWith(newDeliveries[i].location, segments)))
        {
            cerr << "Bad coordinate!" << endl;
            return BAD_COORD;
        }
        
        b = &newDeliveries[i].location;
        if (router.generatePointToPointRoute(*a, *b, route, totalDist) != DELIVERY_SUCCESS) return NO_ROUTE;
        
        double distDownStreet = 0;
        StreetSegment firstStreetSeg = route.front();
        StreetSegment prevStreetSeg = route.front();
        
        list<StreetSegment>::const_iterator it = route.begin();
        while (it != route.end())
        {
            if (it->name == prevStreetSeg.name)
            {
                distDownStreet += distanceEarthMiles(it->start, it->end);
            }
            
            else
            {
                DeliveryCommand proceed;
                double angle = angleOfLine(firstStreetSeg);
                string dir = getDirectionForProceedCmd(angle);
                proceed.initAsProceedCommand(dir, firstStreetSeg.name, distDownStreet);
                commands.push_back(proceed);
                
                totalDistanceTravelled += distDownStreet;
                firstStreetSeg = *it;
                
                DeliveryCommand turn;
                angle = angleBetween2Lines(prevStreetSeg, *it);
                int dirCmd = getDirectionForTurnCmd(angle);
                if (dirCmd != NO_TURN)
                {
                    if (dirCmd == LEFT_TURN)
                    {
                        turn.initAsTurnCommand("left", it->name);
                    }
                    if (dirCmd == RIGHT_TURN)
                    {
                        turn.initAsTurnCommand("right", it->name);
                    }
                    commands.push_back(turn);
                }
                distDownStreet = distanceEarthMiles(it->start, it->end);
            }
            prevStreetSeg = *it;
            ++it;
        }
        
        DeliveryCommand proceedToDelivery;
        double angle = angleOfLine(firstStreetSeg);
        string dir = getDirectionForProceedCmd(angle);
        proceedToDelivery.initAsProceedCommand(dir, firstStreetSeg.name, distDownStreet);
        commands.push_back(proceedToDelivery);
        totalDistanceTravelled += distDownStreet;
        
        DeliveryCommand deliver;
        deliver.initAsDeliverCommand(newDeliveries[i].item);
//        cerr << "Delivered " << deliver.description() << endl;
        commands.push_back(deliver);

        a = &newDeliveries[i].location;
    }
    
    // go home
    std::list<StreetSegment> homeRoute;
    if (router.generatePointToPointRoute(newDeliveries[newDeliveries.size()-1].location, depot, homeRoute, totalDist) != DELIVERY_SUCCESS) return NO_ROUTE;
    
    double distDownStreet = 0;
    StreetSegment firstStreetSeg = homeRoute.front();
    StreetSegment prevStreetSeg = homeRoute.front();
    
    list<StreetSegment>::const_iterator it = homeRoute.begin();
    while (it != homeRoute.end())
    {
        if (it->name == prevStreetSeg.name)
        {
            distDownStreet += distanceEarthMiles(it->start, it->end);
        }
        
        else
        {
            DeliveryCommand d1;
            double angle = angleOfLine(firstStreetSeg);
            string dir = getDirectionForProceedCmd(angle);
            d1.initAsProceedCommand(dir, firstStreetSeg.name, distDownStreet);
            commands.push_back(d1);
            
            firstStreetSeg = *it;
            totalDistanceTravelled += distDownStreet;
            
            DeliveryCommand d2;
            angle = angleBetween2Lines(*it, prevStreetSeg);
            int dirCmd = getDirectionForTurnCmd(angle);
            if (dirCmd != NO_TURN)
            {
                if (dirCmd == LEFT_TURN)
                {
                    d2.initAsTurnCommand("left", it->name);
                }
                if (dirCmd == RIGHT_TURN)
                {
                    d2.initAsTurnCommand("right", it->name);
                }
                commands.push_back(d2);
            }
            distDownStreet = distanceEarthMiles(it->start, it->end);
        }
        
        if (*it == homeRoute.back())
        {
            DeliveryCommand proceedHome;
            double angle = angleOfLine(firstStreetSeg);
            string dir = getDirectionForProceedCmd(angle);
            proceedHome.initAsProceedCommand(dir, firstStreetSeg.name, distDownStreet);
            commands.push_back(proceedHome);
            totalDistanceTravelled += distDownStreet;
        }
        
        prevStreetSeg = *it;
        ++it;
    }
    
    return DELIVERY_SUCCESS;  // Delete this line and implement this function correctly
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
