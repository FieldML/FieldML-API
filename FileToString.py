#Converts the given file into a single constant C string. Used mainly to generate the built-in library and XSD
#strings for FieldML.

import sys

def processFile( filename ):
  rawFile = open( filename )

  for line in rawFile:

    line = line.rstrip()
    line = line.replace( "\"", "\\\"" )
    print line + " \\"

  rawFile.close()

processFile( sys.argv[1] )
