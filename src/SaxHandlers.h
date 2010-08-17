/* \file
 * $Id$
 * \author Caton Little
 * \brief 
 *
 * \section LICENSE
 *
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is FieldML
 *
 * The Initial Developer of the Original Code is Auckland Uniservices Ltd,
 * Auckland, New Zealand. Portions created by the Initial Developer are
 * Copyright (C) 2010 the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 */

#ifndef H_SAX_HANDLERS
#define H_SAX_HANDLERS

#include <string>
#include <libxml/SAX.h>

#include "fieldml_sax.h"

class SaxAttribute
{
public:
    const xmlChar *attribute;
    const char *prefix;
    const char *URI;
    const char *value;
};


class SaxAttributes
{
private:
    SaxAttribute *attributes;
    int count;

public:
    SaxAttributes( const int attributeCount, const xmlChar ** attributes );
    ~SaxAttributes();

    const char *getAttribute( const xmlChar *name );
    bool getBooleanAttribute( const xmlChar *attribute );
    FmlObjectHandle getObjectAttribute( FmlHandle region, const xmlChar *attribute, FieldmlHandleType type );
};


class ObjectListHandler
{
public:
    virtual void onObjectListEntry( FmlObjectHandle handle, int listId ) = NULL;
};


class CharacterBufferHandler
{
public:
    virtual void onCharacterBuffer( const char *buffer, int count, int id ) = NULL;
};


class IntObjectMapHandler
{
public:
    virtual void onIntObjectMapEntry( int key, FmlObjectHandle value, int mapId ) = NULL;
};


class SaxHandler
{
public:
    const xmlChar * const tagName;

    SaxHandler( const xmlChar *_tagName );

    virtual ~SaxHandler();

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes ) = NULL;

    virtual SaxHandler *getParent() = NULL;
    
    virtual void onCharacters( const xmlChar *xmlChars, int count );
};


struct SaxContext
{
    SaxHandler *handler;

    FmlObjectHandle currentObject;

    std::string source;

    FieldmlRegion *region;
};


class RootSaxHandler :
    public SaxHandler
{
public:
    SaxContext * const context;

    RootSaxHandler( const xmlChar *tagName, SaxContext *_context );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *getParent();
};


class FieldmlSaxHandler :
    public SaxHandler
{
private:
    RootSaxHandler * const parent;

public:
    FieldmlSaxHandler( const xmlChar *tagName, RootSaxHandler *_parent );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );

    virtual RootSaxHandler *getParent();
};


class RegionSaxHandler :
    public SaxHandler
{
private:
    FieldmlSaxHandler * const parent;

    FmlHandle region;

public:
    RegionSaxHandler( const xmlChar *tagName, FieldmlSaxHandler *_parent, SaxAttributes &attributes, SaxContext *context );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );

    virtual FieldmlSaxHandler *getParent();

    FmlHandle getRegion();
};


class FieldmlObjectSaxHandler :
    public SaxHandler
{
protected:
    RegionSaxHandler * const parent;

public:
    FmlObjectHandle handle;

    FieldmlObjectSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );

    virtual RegionSaxHandler *getParent();

    FmlHandle getRegion();
};


class ContinuousDomainSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    ContinuousDomainSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class EnsembleDomainSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    EnsembleDomainSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class MeshDomainSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    MeshDomainSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class ContinuousReferenceSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    ContinuousReferenceSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class EnsembleParametersSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
        EnsembleParametersSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

        virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class ContinuousParametersSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    ContinuousParametersSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class ContinuousVariableSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    ContinuousVariableSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class EnsembleVariableSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    EnsembleVariableSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class ContinuousPiecewiseSaxHandler :
    public FieldmlObjectSaxHandler, IntObjectMapHandler
{
public:
    ContinuousPiecewiseSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );

    virtual void onIntObjectMapDefault( FmlObjectHandle object, int mapId );

    virtual void onIntObjectMapEntry( int key, FmlObjectHandle object, int mapId );
};


class ContinuousAggregateSaxHandler :
    public FieldmlObjectSaxHandler, IntObjectMapHandler
{
public:
    ContinuousAggregateSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );

    virtual void onIntObjectMapDefault( FmlObjectHandle object, int mapId );

    virtual void onIntObjectMapEntry( int key, FmlObjectHandle object, int mapId );
};


class ObjectMemberSaxHandler :
    public SaxHandler
{
public:
    FieldmlObjectSaxHandler * const parent;

    ObjectMemberSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );

    virtual FieldmlObjectSaxHandler *getParent();
};


class ContiguousEnsembleBoundsHandler :
    public ObjectMemberSaxHandler
{
public:
    ContiguousEnsembleBoundsHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class MeshShapesSaxHandler :
    public ObjectMemberSaxHandler
{
public:
    MeshShapesSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class MeshConnectivitySaxHandler :
    public ObjectMemberSaxHandler
{
public:
    MeshConnectivitySaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class AliasesSaxHandler :
    public ObjectMemberSaxHandler
{
public:
    AliasesSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
};


class SemidenseSaxHandler :
    public ObjectMemberSaxHandler, ObjectListHandler, CharacterBufferHandler
{
private:
    void onFileData( SaxAttributes &attributes );

public:
    SemidenseSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
    
    virtual void onObjectListEntry( FmlObjectHandle listEntry, int listId );

    virtual void onCharacterBuffer( const char *buffer, int count, int id );
};


class ObjectListSaxHandler :
    public SaxHandler
{
private:
    const FmlHandle region;
    ObjectListHandler * const handler;
    const int listId;
    SaxHandler *parent;

public:
    ObjectListSaxHandler( SaxHandler *_parent, const xmlChar *tagName, FmlHandle _region, ObjectListHandler *_handler, int _listId );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
    
    SaxHandler *getParent();
};


class CharacterBufferSaxHandler :
    public SaxHandler
{
private:
    CharacterBufferHandler * const handler;
    const int bufferId;
    SaxHandler *parent;

public:
    CharacterBufferSaxHandler( SaxHandler *_parent, const xmlChar *tagName, CharacterBufferHandler *_handler, int _listId );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
    
    virtual void onCharacters( const xmlChar *xmlChars, int count );
    
    SaxHandler *getParent();
};


class CharacterAccumulatorSaxHandler :
    public SaxHandler
{
private:
    CharacterBufferHandler * const handler;
    const int bufferId;
    SaxHandler *parent;
    const char *buffer;
    int count;

public:
    CharacterAccumulatorSaxHandler( SaxHandler *_parent, const xmlChar *tagName, CharacterBufferHandler *_handler, int _listId );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
    
    virtual void onCharacters( const xmlChar *xmlChars, int count );
    
    SaxHandler *getParent();
    
    ~CharacterAccumulatorSaxHandler();
};


class IntObjectMapSaxHandler :
    public SaxHandler
{
private:
    const FmlHandle region;
    IntObjectMapHandler * const handler;
    const int mapId;
    SaxHandler *parent;

public:
    IntObjectMapSaxHandler( SaxHandler *_parent, const xmlChar *tagName, FmlHandle _region, IntObjectMapHandler *_handler, int _mapId );

    virtual SaxHandler *onElementStart( const xmlChar *tagName, SaxAttributes &attributes );
    
    SaxHandler *getParent();
};

#endif //H_SAX_HANDLERS
