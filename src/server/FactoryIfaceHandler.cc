/*
 * Copyright 2011 Nate Koenig & Andrew Howard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
/*
 * Desc: FactoryIfaceHandler Position2d controller.
 * Author: Nathan Koenig
 * Date: 01 Feb 2007
 */

#include "common/Events.hh"
#include "common/Global.hh"
#include "common/XMLConfig.hh"
#include "Model.hh"
#include "World.hh"
#include "common/Exception.hh"
#include "gz.h"
#include "FactoryIfaceHandler.hh"

using namespace gazebo;

////////////////////////////////////////////////////////////////////////////////
// Constructor
FactoryIfaceHandler::FactoryIfaceHandler(World *world)
  : world(world)
{
  // Prefix and suffix for xml files
  this->xmlPrefix = "<?xml version='1.0'?> <gazebo:world xmlns:xi='http://www.w3.org/2001/XInclude' xmlns:gazebo='http://playerstage.sourceforge.net/gazebo/xmlschema/#gz' xmlns:model='http://playerstage.sourceforge.net/gazebo/xmlschema/#model' xmlns:sensor='http://playerstage.sourceforge.net/gazebo/xmlschema/#sensor' xmlns:body='http://playerstage.sourceforge.net/gazebo/xmlschema/#body' xmlns:geom='http://playerstage.sourceforge.net/gazebo/xmlschema/#geom' xmlns:joint='http://playerstage.sourceforge.net/gazebo/xmlschema/#joint' xmlns:interface='http://playerstage.sourceforge.net/gazebo/xmlschema/#interface' xmlns:rendering='http://playerstage.sourceforge.net/gazebo/xmlschema/#rendering' xmlns:renderable='http://playerstage.sourceforge.net/gazebo/xmlschema/#renderable' xmlns:controller='http://playerstage.sourceforge.net/gazebo/xmlschema/#controller' xmlns:physics='http://playerstage.sourceforge.net/gazebo/xmlschema/#physics' >";


  this->xmlSuffix = "</gazebo:world>";

  this->factoryIface = (libgazebo::FactoryIface*)libgazebo::IfaceFactory::NewIface("factory");

  // Create the iface
  try
  {
    this->factoryIface->Create(this->world->GetGzServer(), this->world->GetGzServer()->serverName);
    this->factoryIface->Lock(1); // lock it right away to clear up data
    strcpy((char*)this->factoryIface->data->newModel,"");
    this->factoryIface->Unlock();
  }
  catch (std::string e)
  {
    gzthrow(e);
  }

}

////////////////////////////////////////////////////////////////////////////////
// Destructor
FactoryIfaceHandler::~FactoryIfaceHandler()
{
  if (this->factoryIface)
  {
    this->factoryIface->Close();
    this->factoryIface->Destroy();
    delete this->factoryIface;
    this->factoryIface = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Initialize the controller
void FactoryIfaceHandler::Init()
{
}

////////////////////////////////////////////////////////////////////////////////
// Update the controller
void FactoryIfaceHandler::Update()
{
  // If there is a string, then add the contents to the world
  this->factoryIface->Lock(1);

  if (strcmp((const char*)this->factoryIface->data->newModel,"")!=0)
  {
    std::string xmlString;
    std::string xmlMiddle = (const char*)(this->factoryIface->data->newModel);
    
    // Strip leading <?xml...?> tag, if present, to allow the client to
    // pass the contents of a valid .model file
    std::string xmlVersion = "<?xml version=\"1.0\"?>";
    int i = xmlMiddle.find(xmlVersion);
    if(i >= 0)
      xmlMiddle.replace(i, xmlVersion.length(), "");

    xmlString = this->xmlPrefix + xmlMiddle + this->xmlSuffix;

    // Add the new models into the World
    this->world->InsertEntity( xmlString);

    strcpy((char*)this->factoryIface->data->newModel,"");
  }

  // Attempt to delete a model by name, if the string is present
  if (strcmp((const char*)this->factoryIface->data->deleteEntity,"")!=0)
  {
    const std::string e = (const char*)this->factoryIface->data->deleteEntity;
    event::Events::deleteEntitySignal(e);

    strcpy((char*)this->factoryIface->data->deleteEntity,"");
  }
  this->factoryIface->Unlock();

}
