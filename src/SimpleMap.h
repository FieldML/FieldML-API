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

#ifndef H_SIMPLE_MAP
#define H_SIMPLE_MAP

#include <vector>

template <typename K, typename V> class SimpleMap
{
    typedef std::pair<K,V> PairType;
    
private:
    const V invalidValue;
    bool _hasDefault;
    
    V defaultValue;
    
    std::vector<PairType> pairs;
    
    typename std::vector<PairType>::const_iterator find( K key ) const
    {
        for( ConstIterator i = pairs.begin(); i != pairs.end(); i++ )
        {
            if( i->first == key )
            {
                return i;
            }
        }
        
        return pairs.end();
    }

    typename std::vector<PairType>::iterator find( K key )
    {
        for( Iterator i = pairs.begin(); i != pairs.end(); i++ )
        {
            if( i->first == key )
            {
                return i;
            }
        }
        
        return pairs.end();
    }

public:
    typedef typename std::vector<PairType>::const_iterator ConstIterator;
    typedef typename std::vector<PairType>::iterator Iterator;
    
    SimpleMap( const V _invalidValue ) :
        invalidValue( _invalidValue )
    {
        _hasDefault = false;
        defaultValue = invalidValue;
    }


    int size()
    {
        return pairs.size();
    }
    

    ConstIterator begin()
    {
        return pairs.begin();
    }
    
    
    ConstIterator end()
    {
        return pairs.end();
    }
    
    
    
    const V get( K key, bool allowDefault )
    {
        ConstIterator iter = find( key );
        
        if( iter != end() )
        {
            return iter->second;
        }
        else if( !allowDefault )
        {
            return invalidValue;
        }
        else
        {
            return defaultValue;
        }
    }
    
    
    V set( K key, V value )
    {
        Iterator iter = find( key );
        
        if( iter == end() )
        {
            if( ( value != invalidValue ) && ( value != defaultValue ) )
            {
                pairs.push_back( PairType( key, value ) );
            }
            return invalidValue;
        }
        else
        {
            if( ( value == invalidValue ) || ( value == defaultValue ) )
            {
                pairs.erase( iter );
                return invalidValue;
            }
            else
            {
                V previousValue = iter->second;
                iter->second = value;
                
                return previousValue;
            }
        }
    }
    
    
    const K getKey( int index )
    {
        return pairs[index].first;
    }
    
    
    const V getValue( int index )
    {
        return pairs[index].second;
    }
    
    
    void setDefault( const V _default )
    {
        defaultValue = _default;
        
        _hasDefault = (defaultValue != invalidValue);
    }
    
    
    bool hasDefault()
    {
        return _hasDefault;
    }
    
    
    const V getDefault()
    {
        return defaultValue;
    }
};

#endif // H_SIMPLE_MAP
