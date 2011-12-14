#This script extracts tag and attribute names from the given xsd file and generates C header or source
#declarations/definitions for the corresponding strings. These are expected to be pasted in to string_const.h
#or string_const.cpp.

import sys

def addEntry( prefix, line, stringList ):
  index = line.find( prefix )
  if( index > 0 ):
    endIndex = line.find( "\"", index + len( prefix ) )
    if( endIndex > 0 ):
      entry = line[index+len(prefix):endIndex]
      if( stringList.count( entry ) == 0 ):
        stringList.insert( 0, entry )

def printEntry( entry, suffix, hdrMode ):
  oLine = ""
  oLine += "const xmlChar * const "
  oLine += entry[0].upper()
  for i in range(1,len(entry)):
    c = entry[i]
    if( ( c.upper() == c ) and ( entry[i+1].lower() == entry[i+1] ) ):
      oLine += "_"
    oLine += c.upper()
  oLine += suffix

  if( hdrMode ):
    oLine = "extern " + oLine + ";"
  else:
    if( len( oLine ) < 56 ):
      oLine = oLine.ljust( 56 )
    oLine += "= (const xmlChar* const)\"" + entry + "\";"
  
  print oLine

def processFile( filename, hdrMode ):
  rawFile = open( filename )

  elements = []
  attributes = []

  for line in rawFile:
    line = line.rstrip()

    addEntry( "xs:element name=\"", line, elements )

    addEntry( "xs:attribute name=\"", line, attributes )

  for e in elements[0:]:
    printEntry( e, "_TAG", hdrMode )

  print
  print

  for a in attributes[0:]:
    printEntry( a, "_ATTRIB", hdrMode )

  print
    
  rawFile.close()

processFile( sys.argv[1], sys.argv[2] == "True" )
