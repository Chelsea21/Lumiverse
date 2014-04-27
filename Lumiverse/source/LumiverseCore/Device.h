#ifndef _DEVICE_H_
#define _DEVICE_H_

#pragma once

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <sstream>

#include "LumiverseCoreConfig.h"
#include "Logger.h"
#include "LumiverseType.h"
#include "types/LumiverseFloat.h"
#include "types/LumiverseEnum.h"
#include "lib/libjson/libjson.h"
using namespace std;

namespace Lumiverse {
 
  // A Device in Lumiverse maintains information about a lighting device.
  // This class is meant to hold information about different parameters in
  // an Lumiverse friendly way. Conversion to network values happens in a
  // different class to separate the abstract representation of a device
  // from the practical network control details.
  class Device
  {
  public:
    // Default constructor. Every device needs an id, channel, and type.
    // May in the future pull default parameter map from a database of known
    // fixture types.
    Device(string id, unsigned int channel, string type);

    // Make a device given a JSON node
    Device(string id, const JSONNode data);

    // Destructor. Obviously.
    ~Device();

    // string override
    std::ostream & operator<< (std::ostream &str) {
      str << toString();
      return str;
    };

    // Accessors for id
    inline string getId() { return m_id; }

    // Accessors for channel
    inline unsigned int getChannel() { return m_channel; }
    inline void setChannel(unsigned int newChan) { m_channel = newChan; }

    // Accesors for type
    inline string getType() { return m_type; }
    inline void setType(string newType) { m_type = newType; }

    // Gets a parameter value. Returns false if no parameter with the given name exists.
    // Returns true with the parameter value in val if successful.
    bool getParam(string param, float& val);

    // Returns a pointer to the raw LumiverseType data associated with a parameter
    LumiverseType* getParam(string param);

    // Sets a parameter and returns false if parameter changed does not exist prior to set.
    // Can give arbitrary data with this overload.
    bool setParam(string param, LumiverseType* val);

    // Sets a parameter and returns false if parameter changed does not exist prior to set.
    // Returns false otherwise.
    bool setParam(string param, float val);

    // Sets an enumeration.
    // Returns false if the parameter changed does not exist prior to set. If the parameter doesn't
    // exist, it's not actually going to be very useful as an enum so we don't actually create it
    // because we don't know what the enumeration actually consists of.
    // If val2 is not set, the tweak value isn't passed to the enumeration, allowing it to do default behavior.
    bool setParam(string param, string val, float val2 = -1.0f);

    // Will need additional overloads for each new type. Which kinda sucks.

    // Simply returns true if the parameter exists.
    bool paramExists(string param);

    // Resets the values in the parameters to 0 (or equivalent default)
    void clearParamValues();

    // Returns number of parameters in the device.
    unsigned int numParams();

    // Returns list of parameters in the device.
    vector<string> getParamNames();

    // Gets the metadata for a given key. Returns false if no key exists.
    // Returns the value in val otherwise.
    bool getMetadata(string key, string& val);

    // Sets metadata and returns true if metadata key does not exist prior to set.
    // Returns false otherwise.
    bool setMetadata(string key, string val);

    // Resets metadata values to "" but leaves the keys intact.
    void clearMetadataValues();

    // Empties everything in the metadata hash: keys and values. All gone.
    void clearAllMetadata();

    // Returns the number of metadata keys.
    unsigned int numMetadataKeys();

    // Gets list of metadata keys the device has values for.
    vector<string> getMetadataKeyNames();

    // Resets all parameters to default values
    void reset();

    // Converts the device into a string.
    // Data will be output in JSON format
    string toString();

    // Converts the device to a JSONNode.
    JSONNode toJSON();

    // Gets the raw map of parameters to type
    // User shouldn't modify this map directly.
    const map<string, LumiverseType*>* getRawParameters() { return &m_parameters; }
  private:
    // Note that this is private because changing the unique ID after creation
    // can lead to a number of unintended side effects (DMX patch doesn't work,
    // Rig can't find the device, other indexes may fail). This is a little annoying,
    // but in the big scheme of devices and rigs, the ID shouldn't change after initial
    // creation. If the user needs a different display name, the metadata fields say hi.
    // In ACN terms, this would be the CID (if I remember this right), which is unique and
    // immutable for each device).
    void setId(string newId) { m_id = newId; }

    // Takes parsed JSON data and makes a device.
    void loadJSON(const JSONNode data);

    // Loads the parameters of the device from JSON data.
    void loadParams(const JSONNode data);

    // Serializes the parameters into a JSON node
    JSONNode parametersToJSON();

    // Serializes the metadata to a JSON node
    JSONNode metadataToJSON();

    // Unique identifier for the device.
    // Note that while you can use any characters you want in this, you really shouldn't
    // use special characters such as @#$%^=()[]/{} etc.
    // TODO: This should be built in to the set ID function at some point
    // Uniqueness isn't quite enforceable at the device level.
    string m_id;

    // Channel number for the fixture. Does not have to be unique.
    unsigned int m_channel;

    // Device type name. "Source Four ERS" for example.
    string m_type;

    // Map for time-varying parameters.
    // Type may change in the future as more specialized datatypes come up.
    map<string, LumiverseType*> m_parameters;

    // Map for static information.
    // User-defined data helps to add search filters and query devices.
    map<string, string> m_metadata;
  };
}

#endif