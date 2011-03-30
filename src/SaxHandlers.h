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
    virtual ~SaxAttributes();

    const char *getAttribute( const xmlChar *name );
    bool getBooleanAttribute( const xmlChar *attribute );
    FmlObjectHandle getObjectAttribute( FmlHandle sessionHandle, const xmlChar *attribute );
};


class CharacterBufferHandler
{
public:
    virtual void onCharacterBuffer( const char *buffer, int count, int id ) = 0;
};


class IntObjectMapHandler
{
public:
    virtual void onIntObjectMapEntry( int key, FmlObjectHandle value, int mapId ) = 0;
};


class SaxHandler
{
public:
    const xmlChar * const elementName;

    SaxHandler( const xmlChar *_elementName );

    virtual ~SaxHandler();

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes ) = 0;

    virtual SaxHandler *getParent() = 0;
    
    virtual void onCharacters( const xmlChar *xmlChars, int count );
    
    virtual FmlHandle getSessionHandle();
    
    virtual FieldmlSession *getSession();
};


struct SaxContext
{
    SaxHandler *handler;

    std::string source;

    FieldmlSession *session;
};


class RootSaxHandler :
    public SaxHandler
{
public:
    SaxContext * const context;

    RootSaxHandler( const xmlChar *elementName, SaxContext *_context );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *getParent();
    
    virtual FmlHandle getSessionHandle();
    
    virtual FieldmlSession *getSession();
};


class FieldmlSaxHandler :
    public SaxHandler
{
private:
    RootSaxHandler * const parent;

public:
    FieldmlSaxHandler( const xmlChar *elementName, RootSaxHandler *_parent );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );

    virtual RootSaxHandler *getParent();
};


class RegionSaxHandler :
    public SaxHandler
{
private:
    FieldmlSaxHandler * const parent;

    FieldmlRegion *region;

public:
    RegionSaxHandler( const xmlChar *elementName, FieldmlSaxHandler *_parent, SaxAttributes &attributes, SaxContext *context );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );

    virtual FieldmlSaxHandler *getParent();

    FieldmlRegion *getRegion();
};


class FieldmlObjectSaxHandler :
    public SaxHandler
{
protected:
    RegionSaxHandler * const parent;

public:
    FmlObjectHandle handle;

    FieldmlObjectSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes ) = 0;

    virtual RegionSaxHandler *getParent();

    FieldmlRegion *getRegion();
};


class ContinuousTypeSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    ContinuousTypeSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class ImportSaxHandler :
    public SaxHandler
{
private:
    RegionSaxHandler *parent;
        
    int importIndex;
    
public:
    ImportSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *getParent();
};


class EnsembleTypeSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    EnsembleTypeSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class MeshTypeSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    MeshTypeSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class ElementSequenceSaxHandler :
    public FieldmlObjectSaxHandler, CharacterBufferHandler
{
public:
    ElementSequenceSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
    
    virtual void onCharacterBuffer( const char *buffer, int count, int id );
};


class AbstractEvaluatorSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    AbstractEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class ExternalEvaluatorSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    ExternalEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class ReferenceEvaluatorSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    ReferenceEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class ParametersSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
        ParametersSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

        virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class ContinuousParametersSaxHandler :
    public FieldmlObjectSaxHandler
{
public:
    ContinuousParametersSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class PiecewiseEvaluatorSaxHandler :
    public FieldmlObjectSaxHandler, IntObjectMapHandler
{
public:
    PiecewiseEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );

    virtual void onIntObjectMapEntry( int key, FmlObjectHandle object, int mapId );
};


class AggregateEvaluatorSaxHandler :
    public FieldmlObjectSaxHandler, IntObjectMapHandler
{
public:
    AggregateEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );

    virtual void onIntObjectMapEntry( int key, FmlObjectHandle object, int mapId );
};


class ObjectMemberSaxHandler :
    public SaxHandler
{
public:
    FieldmlObjectSaxHandler * const parent;

    ObjectMemberSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes ) = 0;

    virtual FieldmlObjectSaxHandler *getParent();
};


class EnsembleElementsHandler :
    public ObjectMemberSaxHandler, CharacterBufferHandler
{
public:
    EnsembleElementsHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
    
    virtual void onCharacterBuffer( const char *buffer, int count, int id );
};


class MeshShapesSaxHandler :
    public ObjectMemberSaxHandler
{
public:
    MeshShapesSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class MeshConnectivitySaxHandler :
    public ObjectMemberSaxHandler
{
public:
    MeshConnectivitySaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class BindsSaxHandler :
    public ObjectMemberSaxHandler
{
public:
    BindsSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class VariablesSaxHandler :
    public ObjectMemberSaxHandler
{
public:
    VariablesSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
};


class SemidenseSaxHandler :
    public ObjectMemberSaxHandler, CharacterBufferHandler
{
private:
    void onFileData( SaxAttributes &attributes );

public:
    SemidenseSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
    
    virtual void onCharacterBuffer( const char *buffer, int count, int id );
};


class IndexEvaluatorListSaxHandler :
    public SaxHandler
{
private:
    const FieldmlRegion *region;
    SemidenseSaxHandler * const handler;
    const int isSparse;
    SemidenseSaxHandler *parent;

public:
    IndexEvaluatorListSaxHandler( SemidenseSaxHandler *_parent, const xmlChar *elementName, FieldmlRegion *_region, SemidenseSaxHandler *_handler, int _listId );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
    
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
    CharacterBufferSaxHandler( SaxHandler *_parent, const xmlChar *elementName, CharacterBufferHandler *_handler, int _listId );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
    
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
    CharacterAccumulatorSaxHandler( SaxHandler *_parent, const xmlChar *elementName, CharacterBufferHandler *_handler, int _listId );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
    
    virtual void onCharacters( const xmlChar *xmlChars, int count );
    
    SaxHandler *getParent();
    
    virtual ~CharacterAccumulatorSaxHandler();
};


class IntObjectMapSaxHandler :
    public SaxHandler
{
private:
    const xmlChar * entryTagName;
    const FieldmlRegion *region;
    IntObjectMapHandler * const handler;
    const int mapId;
    SaxHandler *parent;

public:
    IntObjectMapSaxHandler( SaxHandler *_parent, const xmlChar *elementName, const xmlChar * _entryTagName, FieldmlRegion *_region, IntObjectMapHandler *_handler, int _mapId );

    virtual SaxHandler *onElementStart( const xmlChar *elementName, SaxAttributes &attributes );
    
    SaxHandler *getParent();
};

#endif //H_SAX_HANDLERS
