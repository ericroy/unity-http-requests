package main

// #cgo CFLAGS: -DUHR_CGO -I../../../include
// #include <UnityHttpRequests.h>
import "C"

import (
	"encoding/binary"
	"errors"
	"unicode/utf16"
	"unicode/utf8"
	"unsafe"
)

const (
	replacementChar = '\uFFFD' // Unicode replacement character
)

const (
	// 0xd800-0xdc00 encodes the high 10 bits of a pair.
	// 0xdc00-0xe000 encodes the low 10 bits of a pair.
	// the value is those 20 bits plus 0x10000.
	surr1 = 0xd800
	surr2 = 0xdc00
	surr3 = 0xe000
)

// Returns value, new offset, error
func readUInt32(b []byte, offset uint) (uint32, uint, error) {
	if len(b) < offset+4 {
		return 0, offset, errors.New("Cannot read uint32, out of range")
	}
	return *(*uint32)(unsafe.Pointer(&b[offset])), offset + 4, nil
}

// Returns new offset, error
func writeUInt32(b []byte, offset uint, u uint32) (uint, error) {
	if len(b) < 4 {
		return offset, errors.New("Cannot write uint32, out of range")
	}
	*(*uint32)(unsafe.Pointer(&b[offset])) = u
	return offset + 4, nil
}

// https://stackoverflow.com/a/23329386
// Returns the number of bytes in the UTF8 encoding of the provided
// UCS2 string, without actually doing the transcoding.
func measureUCS2StringAsUTF8(sr C.UHR_StringRef) uint {
	var encodedLen uint = sr.length
	for i := sr.length - 1; i >= 0; i-- {
		code := sr.characters[i]
		if code > 0x7f && code <= 0x7ff {
			encodedLen++
		} else if code > 0x7ff && code <= 0xffff {
			encodedLen += 2
		}
		// trail surrogate
		if code >= 0xDC00 && code <= 0xDFFF {
			i--
		}
	}
	return encodedLen
}

func decodeUCS2CodePoint(r uint16) (rune, error) {
	if r < surr1 || r > surr3 {
		// normal rune
		return rune(r), nil
	}
	return replacementChar, errors.New("Surrogate sequence found in what was supposed to be a UCS2 code point")
}

// https://golang.org/src/strings/builder.go#L45
// Returns string, new offset, error
func stringFromBytesNoCopy(b []byte, offset uint, length uint) (string, uint, error) {
	if uint(len(b)) < offset+length {
		return "", offset, errors.New("Cannot reference bytes as string, out of range")
	}
	return *(*string)(unsafe.Pointer(&b[offset : offset+length])), offset + length, nil
}

// Buffer must be large enough to accept all bytes of the utf8 encoding.  Measure first!
// Returns new offset, error
func encodeUCS2StringAsUTF8(b []byte, offset uint, sr C.UHR_StringRef) (uint, error) {
	var i uint
	slice := (*[1 << 30]uint16)(unsafe.Pointer(sr.characters))[:uint(sr.length)]
	for _, codePoint := range slice {
		r, err := decodeUCS2CodePoint(codePoint)
		if err != nil {
			return offset + i, err
		}
		i += utf8.EncodeRune(b[offset+i:], r)
	}
	return offset + i, nil
}

// Align up to 4 byte boundary
// Returns new offset
func align4(offset uint) uint {
	if offset&0b11 != 0 {
		offset += 4 - (offset & 0b11)
	}
	return offset
}

func serializeRequest(
	recycledBuffer []byte,
	url C.UHR_StringRef,
	method C.UHR_Method,
	headers *C.UHR_Header,
	headersCount C.uint32_t,
	body *C.char,
	bodyLength C.uint32_t) ([]byte, error) {

	// method, url len, url bytes
	urlLen := measureUCS2StringAsUTF8(url)
	var size uint = 4 + 4 + uint(urlLen)
	size = align4(size)

	size += 4 // headers count
	for i := 0; i < headersCount; i++ {
		// header name len, header name bytes
		size += 4 + measureUCS2StringAsUTF8(headers[i].name)
		size = align4(size)

		// header value len, header value bytes
		size += 4 + measureUCS2StringAsUTF8(headers[i].value)
		size = align4(size)
	}

	// body len, body bytes
	size += 4 + bodyLength

	var buffer []byte
	if cap(recycledBuffer) < size {
		buffer = make([]byte, 0, size)
	} else {
		buffer = recycledBuffer[:size]
	}

	// Write method
	offset, err := writeUInt32(buffer, offset, uint32(method))
	if err != nil {
		return buffer, err
	}

	// Write url length
	offset, err = writeUInt32(buffer, offset, urlLen)
	if err != nil {
		return buffer, err
	}

	// Write utf8 url
	offset, err = encodeUCS2StringAsUTF8(buffer, offset, url)
	if err != nil {
		return buffer, err
	}
	offset = align4(offset)

	// Write headers count
	offset, err = writeUInt32(buffer, offset, headersCount)
	if err != nil {
		return buffer, err
	}

	for i := 0; i < headersCount; i++ {
		// Write header name length
		headerNameLen := measureUCS2StringAsUTF8(headers[i].name)
		offset, err = writeUInt32(buffer, offset, headerNameLen)
		if err != nil {
			return buffer, err
		}

		// Write utf8 header name
		offset, err = encodeUCS2StringAsUTF8(buffer, offset, headers[i].name)
		if err != nil {
			return buffer, err
		}
		offset = align4(offset)

		// Write header value length
		headerValueLen := measureUCS2StringAsUTF8(headers[i].value)
		offset, err = writeUInt32(buffer, offset, headerValueLen)
		if err != nil {
			return buffer, err
		}

		// Write utf8 header value
		offset, err = encodeUCS2StringAsUTF8(buffer, offset, headers[i].value)
		if err != nil {
			return buffer, err
		}
		offset = align4(offset)
	}

	// Write body length
	offset, err = writeUInt32(buffer, offset, bodyLength)
	if err != nil {
		return buffer, err
	}

	// Write body data
	bodySlice := (*[1 << 30]byte)(unsafe.Pointer(body))[:uint(bodyLength)]
	buffer = append(buffer, bodySlice...)

	// Sanity check
	if uint(len(buffer)) != size {
		return buffer, errors.New("Encoding error, result was not the expected number of bytes")
	}

	return buffer, nil
}

func deserializeRequest(
	buffer []byte,
	onMethod func(method uint32),
	onURL func(url string),
	onHeader func(name, value string),
	onBody func(body []byte)) error {

	method, offset, err := readUInt32(buffer, offset)
	if err != nil {
		return err
	}
	onMethod(method)

	urlLen, offset, err := readUInt32(buffer, offset)
	if err != nil {
		return err
	}

	url, offset, err := stringFromBytesNoCopy(buffer, offset, urlLen)
	if err != nil {
		return err
	}
	onURL(url)
	offset = align4(offset)

	headersCount, offset, err := readUInt32(buffer, offset)
	if err != nil {
		return err
	}

	for i := 0; i < headersCount; i++ {
		headerNameLen, offset, err := readUInt32(buffer, offset)
		if err != nil {
			return err
		}

		headerName, offset, err := stringFromBytesNoCopy(buffer, offset, headerNameLen)
		if err != nil {
			return err
		}
		offset = align4(offset)

		headerValueLen, offset, err := readUInt32(buffer, offset)
		if err != nil {
			return err
		}

		headerValue, offset, err := stringFromBytesNoCopy(buffer, offset, headerValueLen)
		if err != nil {
			return err
		}
		offset = align4(offset)

		onHeader(headerName, headerValue)
	}

	bodyLen, offset, err := readUInt32(buffer, offset)
	if err != nil {
		return err
	}

	body, offset, err := stringFromBytesNoCopy(buffer, offset, bodyLen)
	if err != nil {
		return err
	}
	onBody(body)

	return nil
}
