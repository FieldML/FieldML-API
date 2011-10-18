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

#include <cstddef>

#include "SimpleBitset.h"

using namespace std;

static const int BITS_PER_CHUNK = 256;

static const int BITS_PER_INT = 32;

static const int INTS_PER_CHUNK = BITS_PER_CHUNK / BITS_PER_INT; 

/**
 * BitChunk - A helper class we don't want to expose to the outside world.
 */
class BitChunk
{
public:
    int bitCount;
    
    BitChunk( int _firstBit );
    
    void set( int bitNumber, bool state );
    
    bool get( int bitNumber );
    
    void clear();
    
    const int firstBit;
    
    unsigned int bits[INTS_PER_CHUNK];
};


BitChunk::BitChunk( int _firstBit ) :
    firstBit( _firstBit )
{
    clear();
}


void BitChunk::set( int bitNumber, bool state )
{
    int chunkBit = bitNumber & (BITS_PER_CHUNK-1);
    
    int chunkInt = chunkBit / BITS_PER_INT;
    
    int intBit = chunkBit & (BITS_PER_INT-1);
    
    bool oldState = ( bits[chunkInt] & ( 1 << intBit ) ) != 0;
    
    if( state && !oldState )
    {
        bits[chunkInt] |= ( 1 << intBit );
        bitCount++;
    }
    else if( oldState && !state )
    {
        bits[chunkInt] &= ~( 1 << intBit );
        bitCount--;
    }
}


bool BitChunk::get( int bitNumber )
{
    int chunkBit = bitNumber & (BITS_PER_CHUNK-1);
    
    int chunkInt = chunkBit / BITS_PER_INT;
    
    int intBit = chunkBit & (BITS_PER_INT-1);
    
    return ( bits[chunkInt] & ( 1 << intBit ) ) != 0;
}


void BitChunk::clear()
{
    bitCount = 0;

    for( int i = 0; i < INTS_PER_CHUNK; i++ )
    {
        bits[i] = 0;
    }
}


SimpleBitset::SimpleBitset()
{
}


SimpleBitset::~SimpleBitset()
{
    BitChunk *chunk;
    for( list<BitChunk *>::iterator i = chunks.begin(); i != chunks.end(); i++ )
    {
        chunk = *i;
        delete chunk;
    }
}


BitChunk *SimpleBitset::getChunk( int bitNumber, bool create )
{
    if( bitNumber < 0 )
    {
        return NULL;
    }
    
    BitChunk *chunk = NULL;
    list<BitChunk *>::iterator i;
    for( i = chunks.begin(); i != chunks.end(); i++ )
    {
        chunk = *i;
        if( ( bitNumber >= chunk->firstBit ) && ( bitNumber < chunk->firstBit + BITS_PER_CHUNK ) )
        {
            return chunk;
        }
        if( bitNumber < chunk->firstBit )
        {
            break;
        }
    }
    
    if( create )
    {
        int chunkFirst = bitNumber & ~(BITS_PER_CHUNK-1); 
        chunk = new BitChunk( chunkFirst );
        if( i == chunks.begin() )
        {
            chunks.push_front( chunk );
        }
        else
        {
            chunks.insert( i, chunk );
        }
    }
    
    return chunk;
}


BitChunk *SimpleBitset::getNextChunk( int bitNumber )
{
    if( bitNumber < 0 )
    {
        return NULL;
    }
    
    BitChunk *bestChunk = NULL;
    BitChunk *chunk = NULL;
    for( list<BitChunk *>::iterator i = chunks.begin(); i != chunks.end(); i++ )
    {
        chunk = *i;
        if( chunk->firstBit + BITS_PER_CHUNK - 1 < bitNumber )
        {
            continue;
        }
        else if( ( bestChunk == NULL ) || ( chunk->firstBit < bestChunk->firstBit ) )
        {
            bestChunk = chunk;
        }
    }
    
    return bestChunk;
}


void SimpleBitset::setBit( int bitNumber, bool state )
{
    BitChunk *chunk = getChunk( bitNumber, state );
    if( chunk == NULL )
    {
        //Asked to set a non-existant bit to 'false'. That's easy.
        return;
    }
    
    chunk->set( bitNumber, state );
}


bool SimpleBitset::getBit( int bitNumber )
{
    BitChunk *chunk = getChunk( bitNumber, false );
    if( chunk == NULL )
    {
        return false;
    }
    
    return chunk->get( bitNumber );
}


int SimpleBitset::getCount()
{
    int count = 0;
    BitChunk *chunk;
    for( list<BitChunk *>::iterator i = chunks.begin(); i != chunks.end(); i++ )
    {
        chunk = *i;
        count += chunk->bitCount;
    }
    
    return count;
}


void SimpleBitset::clear()
{
    BitChunk *chunk;
    for( list<BitChunk *>::iterator i = chunks.begin(); i != chunks.end(); i++ )
    {
        chunk = *i;
        chunk->clear();
    }
    
}


int SimpleBitset::getNextTrueBit( int bitNumber )
{
    BitChunk *chunk;
    
    while( true )
    {
        chunk = getNextChunk( bitNumber );
        
        if( chunk == NULL )
        {
            return -1;
        }
        
        if( chunk->bitCount == 0 )
        {
            continue;
        }
        
        for( ; bitNumber < chunk->firstBit + BITS_PER_CHUNK; bitNumber++ )
        {
            if( chunk->get( bitNumber ) )
            {
                return bitNumber;
            }
        }
    }
}


int SimpleBitset::getTrueBit( int bitCount )
{
    //This code is a little inefficient, but should do for now.
    int bitCounter = 0;
    BitChunk *chunk = NULL;
    BitChunk *goodChunk = NULL;
    for( list<BitChunk *>::iterator i = chunks.begin(); i != chunks.end(); i++ )
    {
        chunk = *i;
        if( ( bitCounter < bitCount ) && ( chunk->bitCount + bitCounter >= bitCount ) )
        {
            goodChunk = chunk;
            break;
        }
        bitCounter += chunk->bitCount;
    }
    
    if( goodChunk == NULL )
    {
        return -1;
    }
    
    bitCount -= bitCounter;
    int bitNumber = chunk->firstBit;
    
    for( ; bitNumber < chunk->firstBit + BITS_PER_CHUNK; bitNumber++ )
    {
        if( chunk->get( bitNumber ) )
        {
            bitCount--;
            if( bitCount == 0 )
            {
                return bitNumber;
            }
        }
    }
    
    return -1;
}
