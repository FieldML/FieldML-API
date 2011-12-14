#This script extracts FieldML API function definitions and declarations from a source/header file pair
#and makes sure that all definitions are declared, and all declarations are defined. It also checks
#that return types and argument types and names match.

import sys
import re

def processHeader( filename ):
  rawFile = open( filename )

  declarations = []

  for line in rawFile:
    line = line.rstrip()

    m = re.search( '(^.* (Fieldml_[^\(].+)\(.*\))\;$', line )
    if( m != None ):
      declarations.append( ( m.group( 1 ), m.group( 2 ) ) )
      
  rawFile.close()
  
  return declarations

def processSource( filename ):
  rawFile = open( filename )

  definitions = []

  for line in rawFile:
    line = line.rstrip()

    m = re.search( '(^[^ ].* (Fieldml_[^\(].+)\(.*\))$', line )
    if( m != None ):
      definitions.append( ( m.group( 1 ), m.group( 2 ) ) )
      
  rawFile.close()
  
  return list( set( definitions ) )

def processFiles( sourceFilename, headerFilename ):

  print "Checking " + sourceFilename + " against " + headerFilename + "."

  definitions = processSource( sourceFilename )
  declarations = processHeader( headerFilename )
  
  #Yes, it's n-squared. Small n, no problem.
  for declaration in reversed( declarations ):
    for definition in definitions:
      if( declaration == definition ):
        declarations.remove( declaration )
        definitions.remove( declaration )
        break
      elif( declaration[1] == definition[1] ):
        print "Mismatch for \"" + definition[1] + "\""
        declarations.remove( declaration )
        definitions.remove( definition )
        break

  for definition in definitions:
    print "Definition without declaration: \"" + definition[1] + "\""

  for declaration in declarations:
    print "Declaration without definition: \"" + declaration[1] + "\""
    
  print "Done."

processFiles( "core\\src\\fieldml_api.cpp", "core\\src\\fieldml_api.h" )

processFiles( "io\\src\\FieldmlIoApi.cpp", "io\\src\\FieldmlIoApi.h" )
