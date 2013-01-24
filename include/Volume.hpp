/**
  * @file include/Volume.hpp 
  * @author 
  * 
 **/
#ifndef _VOLGEN_VOLUME_HPP_
#define _VOLGEN_VOLUME_HPP_

#include <inttypes.h>


#include <string>
#include <list>


namespace volgen {



struct VolumeItem {
    std::string  fullname;
    std::string  name;
    uint64_t     size;
    float        vrat;

    VolumeItem() : size(0), vrat(0.0) {}
};

typedef std::list<VolumeItem> ItemList;



struct Volume {
    std::string  name;
    ItemList     items;
    uint64_t     size;
    float        vtot;

    Volume() : size(0), vtot(0.0) {}

    Volume ( const std::string & vname ) 
        : name(vname),
          size(0), 
          vtot(0.0) 
    {}
};

typedef std::list<Volume*> VolumeList;


}  // namespace


#endif  // _VOLGEN_VOLUME_HPP_

