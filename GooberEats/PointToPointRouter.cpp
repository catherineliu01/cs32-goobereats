#include "provided.h"
#include <list>
using namespace std;

#include "ExpandableHashMap.h"
#include <queue>

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_map;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    m_map = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
//    route.clear();
//    totalDistanceTravelled = 0;
//
//    queue<GeoCoord> q;
//    q.push(start);
//    vector<StreetSegment> hasVisited;
//    ExpandableHashMap<GeoCoord, GeoCoord> locationOfPrevWayPoint;
//
//    while (!q.empty())
//    {
//        GeoCoord current = q.front();
//        q.pop();
//
//        if (current == end)
//        {
//            // reconstruct path
//            cerr << "Found path!" << endl;
//
//            const GeoCoord* b = &end;
//            const GeoCoord* a = locationOfPrevWayPoint.find(end);
//            while (*b != start)
//            {
//                vector<StreetSegment> segs;
//                m_map->getSegmentsThatStartWith(*a, segs);
//                string address = "";
//                StreetSegment s;
//                for (int i = 0; i < segs.size(); i++)
//                {
//                    if (segs[i].end == *b)
//                    {
//                        address = segs[i].name;
//                        s = segs[i];
//                        break;
//                    }
//                }
//                totalDistanceTravelled += distanceEarthMiles(*a, *b);
//                route.push_front(s);
//
//                GeoCoord temp = *a;
//                a = locationOfPrevWayPoint.find(*a);
//                b = &temp;
//            }
//
//            return DELIVERY_SUCCESS;
//        }
//
//        vector<StreetSegment> neighbors;
//        if (!m_map->getSegmentsThatStartWith(current, neighbors))
//        {
//            cerr << "Bad coord!" << endl;
//            return BAD_COORD;
//        }
//
//        for (int i = 0; i < neighbors.size(); i++)
//        {
////            bool dontVisitAgain = false;
////            for (int j = 0; j < hasVisited.size(); j++)
////            {
////                if (neighbors[i] == hasVisited[j])
////                {
////                    cerr << "no backtracking!" << endl;
////                    dontVisitAgain = true;
////                    break;
////                }
////            }
////            if (!dontVisitAgain)
////            {
////                q.push(neighbors[i].end);
////                hasVisited.push_back(neighbors[i]);
////                locationOfPrevWayPoint.associate(neighbors[i].end, current);
////            }
//            GeoCoord* segEnd = locationOfPrevWayPoint.find(neighbors[i].end);
//            if (segEnd == nullptr || *segEnd != current)
//            {
//                locationOfPrevWayPoint.associate(neighbors[i].end, current);
//                q.push(neighbors[i].end);
//            }
//        }
//    }
//
//    return NO_ROUTE;
    
    route.clear();
    totalDistanceTravelled = 0;

    if (start == end)
    {
        return DELIVERY_SUCCESS;
    }

    vector<StreetSegment> segments;
    if (!(m_map->getSegmentsThatStartWith(start, segments) && m_map->getSegmentsThatStartWith(end, segments)))
    {
        cerr << "Bad coordinate!" << endl;
        return BAD_COORD;
    }

    ExpandableHashMap<GeoCoord, GeoCoord> parentMap;
    ExpandableHashMap<GeoCoord, double> gValues;
    ExpandableHashMap<GeoCoord, double> fValues;

    struct compare // greater than comparator - so that we can sort priority_queue by f value
    {
        ExpandableHashMap<GeoCoord, double>* m_fValues;
        compare(ExpandableHashMap<GeoCoord, double>& fValues) { m_fValues = &fValues; }
        bool operator()(const GeoCoord& l, const GeoCoord& r)
        {
            return (*(m_fValues->find(l)) > *(m_fValues->find(r)));
        }
    };

    compare comp(fValues);
    priority_queue<GeoCoord, vector<GeoCoord>, compare> openSet(comp);
    openSet.push(start);
    fValues.associate(start, 0);
    gValues.associate(start, 0);

    GeoCoord current;

    while (!openSet.empty())
    {
        current = openSet.top();
        if (current == end) // reconstruct path
        {
//            cerr << "Found path!" << endl;
            const GeoCoord* b = &end;
            const GeoCoord* a = parentMap.find(end);
            while (*b != start)
            {
                vector<StreetSegment> segs;
                m_map->getSegmentsThatStartWith(*a, segs);
                string address = "";
                StreetSegment s;
                for (int i = 0; i < segs.size(); i++)
                {
                    if (segs[i].end == *b)
                    {
                        address = segs[i].name;
                        s = segs[i];
                        break;
                    }
                }
                totalDistanceTravelled += distanceEarthMiles(*a, *b);
                route.push_front(s);

                GeoCoord temp = *a;
                a = parentMap.find(*a);
                b = &temp;
            }
            return DELIVERY_SUCCESS;
        }

        openSet.pop();
        vector<StreetSegment> segs;
        m_map->getSegmentsThatStartWith(current, segs);
        for (int i = 0; i < segs.size(); i++)
        {
            GeoCoord neighbor = segs[i].end;
            double currentG = *gValues.find(current);
            double tentativeG = currentG + distanceEarthMiles(current, neighbor);

            double* neighborG = gValues.find(neighbor);
            if (neighborG != nullptr)
            {
                if (tentativeG < *neighborG)
                {
                    parentMap.associate(neighbor, current);
                    gValues.associate(neighbor, tentativeG);
                    double h = distanceEarthMiles(neighbor, end);
                    fValues.associate(neighbor, tentativeG + h);
                }
            }
            else
            {
                parentMap.associate(neighbor, current);
                gValues.associate(neighbor, tentativeG);
                double h = distanceEarthMiles(neighbor, end);
                fValues.associate(neighbor, tentativeG + h);

                openSet.push(neighbor);
            }
        }
    }

    cerr << "failure!" << endl;
    return NO_ROUTE;  // Delete this line and implement this function correctly
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
