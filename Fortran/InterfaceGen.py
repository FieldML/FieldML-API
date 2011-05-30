import datetime

class ParseState:
  InHeader, InStartEnum, InEnums, InEndEnum = range(4)
  enumCounter = 0
  functions = []
  defines = []
  enums = []


def expose( names ):
  if( len( names ) == 0 ):
    return

  line = "  PUBLIC " + names[0][0]
  enumCounter = 1
  while( enumCounter < len( names ) ):
    if( len( line ) + len( names[enumCounter][0] ) > 120 ):
      print line + ", &"
      line = "    & "
    else:
      line += ", "
    line += names[enumCounter][0]
    enumCounter += 1

  print line
  print


def declareFunction( name, types, names ):
  line = "    FUNCTION " + name + "("
  if( len( names ) > 0 ):
    line += " " + names[0]
    for n in names[1:]:
      line += ", " + n
  line += " ) &"

  print line
  print "      & BIND(C,NAME=\""+ name + "\")"
#  print "      USE TYPES"
  print "      USE ISO_C_BINDING"

  for i, p in enumerate( types ):
    if( p == "char" ):
      print "      CHARACTER(KIND=C_CHAR) :: " + names[i]
    elif( p == "char*" ):
      print "      CHARACTER(KIND=C_CHAR) :: " + names[i] + "(*)"
    elif( p == "int*" ) or ( p == "double*"):
      print "      TYPE(C_PTR), VALUE :: " + names[i]
    else:
      print "      INTEGER(C_INT), VALUE :: " + names[i]

  print "      INTEGER(C_INT) :: " + name
  print "    END FUNCTION " + name


def declareConstant( name, value ):
  print "  INTEGER(C_INT), PARAMETER :: " + name + " =",
  print value


def writeHeader():
  now = datetime.datetime.now()

  print "!This file was automatically generated from fieldml_api.h on " + now.strftime("%Y-%m-%d %H:%M")
  print "MODULE FIELDML_API"
  print ""
  print "  USE ISO_C_BINDING"
  print ""
  print "  IMPLICIT NONE"
  print ""

def writeFooter():
  print "END MODULE FIELDML_API"

def processFunction( line ):
  #Don't need to process functions returning char* or int*, those aren't usable by Fortran.
  if( ( line.find( "char*" ) == 0 ) or ( line.find( "char *" ) == 0 ) or ( line.find( "const char *" ) == 0 )  ):
    return
  if( ( line.find( "int*" ) == 0 ) or ( line.find( "int *" ) == 0 ) or ( line.find( "const int *" ) == 0 )  ):
    return

  fStart = line.find( " " )
  if( fStart == -1 ):
    return

  fEnd = line.find( "(" )
  if( fEnd == -1 ):
    return

  pEnd = line.find( ")" )
  if( pEnd == -1 ):
    return

  fName = line[ fStart+1 : fEnd ]

  rawParams = line[ fEnd+1 : pEnd-1 ].strip().split()

  types = []
  names = []
  type = ""

  isType = True

  for i, p in enumerate( rawParams ):
    pos = p.find( "," )
    if( pos != -1 ):
      p = p[ 0 : pos ]

    if p == "*":
      type = type + "*"
      continue
    if p == "const":
      continue

    pos = p.find( "*" )
    if( pos == 0 ):
      type = type + "*"
      p = p[ 1 : len(p) ]

    if( isType ):
      type = p.strip()
      isType = False
    else:
      types.append( type )
      names.append( p.strip() )
      isType = True

  ParseState.functions.append( [ fName, types, names ] )


def processDefine( line ):
  pos = line.find( " " )
  if( pos == -1 ):
    return

  name = line[ 0 : pos ].strip()
  value = line [ pos + 1 : ].strip()

  ParseState.defines.append( [ name, value ] )


def processEnums( line ):
  lastPos = 0
  pos = line.find( ",", lastPos )
  while( pos != -1 ):
    name = line[ lastPos : pos ].strip()
    if( len( name ) != 0 ):
      ParseState.enums[len( ParseState.enums ) - 1].append( [ name, ParseState.enumCounter ] )
    ParseState.enumCounter += 1
    lastPos = pos + 1
    pos = line.find( ",", lastPos )
    
  name = line[ lastPos : ].strip()
  if( len( name ) != 0 ):
    ParseState.enums[len( ParseState.enums ) - 1].append( [ name, ParseState.enumCounter ] )
    ParseState.enumCounter += 1

def processLine( line, state ):
  pos = line.find( "//" )
  if( pos != -1 ):
    line = line[ 0 : pos ]
  line = line.strip()

  if( len( line ) == 0 ):
    return state

  if( state == ParseState.InHeader ):

  #Current code assumes all function declarations are single-line
    if( line.find( ");" ) != -1 ):
      processFunction( line )
      return ParseState.InHeader

  #Current code assumes all function declarations are single-line
    if( line.find( "#define " ) == 0 ):
      processDefine( line[ 8 : ] )
      return ParseState.InHeader
    
    if( line.find( "enum" ) == 0 ):
      ParseState.enumCounter = 0
      ParseState.enums.append( [] )
      return processLine( line, ParseState.InStartEnum )

  if( state == ParseState.InStartEnum ):
    pos = line.find( "{" )
    if( pos != -1 ):
      return processLine( line[ pos + 1 : ], ParseState.InEnums )

  if( state == ParseState.InEnums ):
    pos = line.find( "}" )
    if( pos != -1 ):
      processEnums( line[ 0 : pos ] )
      return processLine( line[ pos + 1 : ], ParseState.InEndEnum )
    else:
      processEnums( line )

  if( state == ParseState.InEndEnum ):
    pos = line.find( ";" )
    if( pos != -1 ):
      return ParseState.InHeader

  return state


def processFile():
  headerFile = open( "../src/fieldml_api.h" )

  state = ParseState.InHeader
  inComment = False

  for line in headerFile:
    startSegment = 0

    if( inComment ):
      pos = line.find( "*/" )
      if( pos == -1 ):
        continue
      inComment = False
      startSegment = pos + 2

    #Does not cope with mixing line and block comments.
    endSegment = line.find( "/*", startSegment )
    while( endSegment != -1 ):
      state = processLine( line[ startSegment : endSegment ], state )
      startSegment = line.find( "*/", endSegment + 2 )
      if( startSegment == -1 ):
        break
      endSegment = line.find( "/*", startSegment )

    if( startSegment == -1 ):
      inComment = True
    else:
      state = processLine( line[ startSegment : ], state )

  headerFile.close()


  writeHeader()

  for d in ParseState.defines:
    declareConstant( d[0], d[1] )
    print

  for enum in ParseState.enums:
    for e in enum:
      declareConstant( e[0], e[1] )
    print

  print "  INTERFACE"
  for f in ParseState.functions:
    declareFunction( f[0], f[1], f[2] )
    print
  print "  END INTERFACE"
  print

  expose( ParseState.functions )
  for e in ParseState.enums:
    expose( e )
  expose( ParseState.defines )

  writeFooter()

processFile()
