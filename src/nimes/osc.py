import math
import struct
import string


def _readString(data):
    """Reads the next (null-terminated) block of data
    """
    length = string.find(data, "\0")
    nextData = int(math.ceil((length+1) / 4.0) * 4)
    return (data[0:length], data[nextData:])


def _readBlob(data):
    """Reads the next (numbered) block of data
    """
    length = struct.unpack(">i", data[0:4])[0]
    nextData = int(math.ceil((length) / 4.0) * 4) + 4
    return (data[4:length+4], data[nextData:])


def _readInt(data):
    """Tries to interpret the next 4 bytes of the data
    as a 32-bit integer. """
    if(len(data) < 4):
        print "Error: too few bytes for int", data, len(data)
        rest = data
        integer = 0
    else:
        integer = struct.unpack(">i", data[0:4])[0]
        rest = data[4:]

    return (integer, rest)


def _readTimeTag(data):
    """Tries to interpret the next 8 bytes of the data
    as a TimeTag.
     """
    high, low = struct.unpack(">ll", data[0:8])
    if (high == 0) and (low <= 1):
        time = 0.0
    else:
        time = int(high) + float(low / 1e9)
    rest = data[8:]
    return (time, rest)


def _readFloat(data):
    """Tries to interpret the next 4 bytes of the data
    as a 32-bit float.
    """

    if(len(data) < 4):
        print "Error: too few bytes for float", data, len(data)
        rest = data
        float = 0
    else:
        float = struct.unpack(">f", data[0:4])[0]
        rest = data[4:]

    return (float, rest)

def _readDouble(data):
    """Tries to interpret the next 8 bytes of the data
    as a 64-bit float. 
    """
    
    if(len(data)<8):
        print "Error: too few bytes for double", data, len(data)
        rest = data
        float = 0
    else:
        float = struct.unpack(">d", data[0:8])[0]
        rest  = data[8:]

    return (float, rest)

def decodeOSC(data):
    """Converts a binary OSC message to a Python list.
    """
    table = {"i":_readInt, "f":_readFloat, "s":_readString, "b":_readBlob, "d":_readDouble, "t":_readTimeTag}
    decoded = []
    address,  rest = _readString(data)
    if address.startswith(","):
        typetags = address
        address = ""
    else:
        typetags = ""

    if address == "#bundle":
        time, rest = _readTimeTag(rest)
        decoded.append(address)
        decoded.append(time)
        while len(rest) > 0:
            length, rest = _readInt(rest)
            decoded.append(decodeOSC(rest[:length]))
            rest = rest[length:]

    elif len(rest) > 0:
        if not len(typetags):
            typetags, rest = _readString(rest)
        decoded.append(address)
        decoded.append(typetags)
        if typetags.startswith(","):
            for tag in typetags[1:]:
                value, rest = table[tag](rest)
                decoded.append(value)
        else:
            raise ValueError("OSCMessage's typetag-string lacks the magic ','")

    return decoded
