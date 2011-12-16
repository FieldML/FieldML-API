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

#ifndef H_SIMPLE_TEST
#define H_SIMPLE_TEST

#include <string>
#include <vector>
#include <iostream>
#include <sstream>


/**
 * The base class for all exceptions thrown by tests. These are caught and processed by the framework. 
 */
class SimpleTestException
{
private:
    std::string message;
    
public:
    SimpleTestException( std::string _message, const char *_file, const int _line );
    
    const std::string getMessage();
};


/**
 * Executes and records the results of any tests. An instance of this class is passed to all user-defined
 * test functions.
 */
class SimpleTestRecorder
{
private:
    int passed;
    int failed;
    int excepted;

public:
    SimpleTestRecorder();
    
    
    void reset();
    
    
    void checkAndThrow( int expr, const std::string &message, const char *_file, const int _line );
    
    
    void assert( int expr, const std::string &exprString, const char *_file, const int _line );


    template<typename T> void assertEquals( T const &expected, T const &actual, const std::string &actualName, const char *_file, const int _line )
    {
        std::stringstream message;

        message << "Assert failed on " << actualName << ". Expected " << expected << ", got " << actual << ".";
        checkAndThrow( expected == actual, message.str(), _file, _line );
    }
    

    void assertEquals( const char *const &expected, char *const &actual, const std::string &actualName, const char *_file, const int _line );
    
    
    void assertEquals( const int &expected, const unsigned int &actual, const std::string &actualName, const char *_file, const int _line );


    void assertEquals( const unsigned int &expected, const int &actual, const std::string &actualName, const char *_file, const int _line );

    
    void report();
};



class SimpleTest
{
public:
    static std::vector<SimpleTest*> tests;

    void (*testFunction)( SimpleTestRecorder &__recorder );

    const std::string name;

    SimpleTest( void (*_testFunction)( SimpleTestRecorder &__recorder ), const std::string &_name );
};

//Macro tries to avoid name-collision with SimpleTestRecorder parameter name, as it is hidden from the user.
#define SIMPLE_TEST( name ) \
    static void name( SimpleTestRecorder &__recorder ); \
    static SimpleTest SimpleTest##name( name, #name ); \
    static void name( SimpleTestRecorder &__recorder )

#define SIMPLE_ASSERT( expr ) __recorder.assert( ( expr ), #expr, __FILE__, __LINE__ )

#define SIMPLE_ASSERT_EQUALS( expected, actual ) __recorder.assertEquals( ( expected ), ( actual ), #actual, __FILE__, __LINE__ )

#define SIMPLE_CHECK( expr ) __recorder.check( ( expr ), #expr, __FILE__, __LINE__ )

#define SIMPLE_CHECK_EQUALS( expected, actual ) __recorder.checkEquals( ( expected ), ( actual ) , #actual, __FILE__, __LINE__ )


#endif //H_SIMPLE_TEST
