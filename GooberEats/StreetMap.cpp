#include "provided.h"
#include <string>
#include <vector>
#include <functional>
using namespace std;

#include <iostream>
#include <fstream>

#include "ExpandableHashMap.h" // ahh

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_map;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream infile(mapFile);
    if ( ! infile )                // Did opening the file fail?
    {
        cerr << "Error: Cannot open mapdata.txt!" << endl;
        return false;
    }
    
    string address;
    int count;
    string startLat;
    string startLong;
    string endLat;
    string endLong;
    
    while (infile)
    {
        getline(infile, address);
        infile >> count;
        
        infile.ignore(10000, '\n');
        for (int i = 0; i < count; i++)
        {
            infile >> startLat;
            infile >> startLong;
            infile >> endLat;
            infile >> endLong;
            
            GeoCoord start(startLat, startLong);
            GeoCoord end(endLat, endLong);
            
            vector<StreetSegment>* startVal = m_map.find(start);
            if (startVal != nullptr)
            {
                startVal->push_back(StreetSegment(start, end, address));
            }
            else
            {
                vector<StreetSegment> seg;
                seg.push_back(StreetSegment(start, end, address));
                m_map.associate(start, seg);
            }
            
            vector<StreetSegment>* endVal = m_map.find(end);
            if (endVal != nullptr)
            {
                endVal->push_back(StreetSegment(end, start, address));
            }
            else
            {
                vector<StreetSegment> seg;
                seg.push_back(StreetSegment(end, start, address));
                m_map.associate(end, seg);
            }
        }
        infile.ignore(10000, '\n');
    }
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment>* val = m_map.find(gc);
    if (val == nullptr) return false;
    else
    {
        segs = *val;
        return true;
    }
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
