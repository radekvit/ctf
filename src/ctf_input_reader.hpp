/**
\file input_reader.hpp
\brief Contains the InputReader class.
\author Radek Vit
*/
#ifndef CTF_INPUT_READER_H
#define CTF_INPUT_READER_H

#include <algorithm>
#include <istream>
#include <limits>
#include "ctf_base.hpp"

namespace ctf {

/**
\brief Reads characters from a stream and stores them in a line buffer.

Offers read operations, unget operations, location buffering etc.
Manages read characters and offers character and line retreival.
*/
class InputReader {
 public:
  /**
  \brief Default empty constructor. To put the object in a working state,
  set_stream() must be called.
  */
  InputReader() = default;
  /**
  \brief Constructs InputReader in a valid state.

  \param[in] is The input stream to be used by InputReader.
  \param[in] streamName The name of the input stream.
  */
  InputReader(std::istream& is, const string& streamName = "") { set_stream(is, streamName); }

  /**
  \brief Returns a pointer to the assigned stream.

  \returns The pointer to the assigned stream.
  */
  std::istream* stream() const { return _is; }
  /**
  \brief Returns the assigned stream name.

  \returns The assigned stream name.
  */
  const string& stream_name() const noexcept { return _streamName; }

  /**
  \brief Sets the stream and resets the input buffer.

  \param[in] is The input stream to be used by InputReader.
  \param[in] streamName The name of the input stream.
  */
  void set_stream(std::istream& is, const string& streamName = "") {
    _is = &is;
    _streamName = streamName;
    _inputBuffer.reset();
    _currentLocation = Location{streamName};
  }
  /**
  \brief Gets the next character.

  \returns The next read character.
  */
  int get() {
    int c = 0;
    // cannot get
    if (!_inputBuffer.get(c, _currentLocation)) {
      c = _is->get();
      _inputBuffer.append(c);
      _currentLocation = _inputBuffer.next_location(c, _currentLocation);
    }
    return c;
  }
  /**
  \brief Gets the next character and its location.

  \param[out] location The location of the read character.

  \returns The next read character.
  */
  int get(Location& location) {
    location = _currentLocation;
    return get();
  }
  /**
  \brief Moves the read head N characters back.

  \param[in] rollback How many characters to roll back.
  */
  void unget(size_t rollback = 1) noexcept {
    // rollback
    _currentLocation = _inputBuffer.unget(_currentLocation, rollback);
  }
  /**
  \brief Moves the read head N characters back and sets the rolled back
  location.

  \param[out] location The current location after rollback.
  \param[in] rollback How many characters to roll back.
  */
  void unget(Location& location, size_t rollback = 1) noexcept {
    // rollback
    _currentLocation = _inputBuffer.unget(_currentLocation, rollback);
    location = _currentLocation;
  }

  /**
  \brief Obtain a line from the input.

  \param[in] args Arguments for InputBuffer::get_line

  \returns A string containing all characters on that line.
  */
  template <typename... Args>
  string get_line(Args&&... args) const {
    return _inputBuffer.get_line(std::forward<Args>(args)...);
  }

  /**
  \brief Get the whole input as a string.

  \returns A string containing all read characters.
  */
  string get_all() const { return _inputBuffer.get_all(); }

  /**
  \brief Reset the reader state. This operation resets the internal position.
  */
  void reset() { _currentLocation = Location{_currentLocation.fileName}; }

 private:
  /**
  \brief Buffer class with Location-based indexing.

  Characters are exposed as immutable; only appending characters is possible.
  */
  class InputBuffer {
   public:
    /**
    \brief The eof constant.
    */
    static const int eof = std::char_traits<char>::eof();

    /**
    \brief Constructs the InputBuffer object and resets it.
    */
    InputBuffer() { reset(); }
    /**
    \brief Resets the InputBuffer object.
    */
    void reset() {
      _charBuffer.clear();
      _lineStartBuffer.clear();
      _lineStartBuffer.push_back(0);
      eofLocation_ = std::numeric_limits<size_t>::max();
    }
    /**
    \brief Appends the character to the end of the buffer.

    \param[in] c The character that is appended.
    */
    void append(int c) {
      if (c == eof) {
        eofLocation_ = _charBuffer.size();
        return;
      }
      _charBuffer.push_back(c);
      if (c == '\n') {
        _lineStartBuffer.push_back(_charBuffer.size());
      }
    }

    /**
    \brief Reads a character and moves a location to its next position.

    \param[out] c The read character.
    \param[in,out] location The next location to be read.

    \returns True if a character is present in the buffer or is eof. False
    otherwise.
    */
    bool get(int& c, Location& location) const noexcept {
      auto it = character(location);
      if (it == _charBuffer.end() && _charBuffer.size() == eofLocation_) {
        c = eof;
        return true;
      } else if (it >= line_end(line(location))) {
        return false;
      }
      c = static_cast<int>(*it);
      location = next_location(c, location);
      return true;
    }

    /**
    \brief Returns a line of characters.

    \param[in] row The row to be returned.

    \returns A vector of all characters on that row.
    */
    string get_line(size_t row) const { return {line_begin(row - 1), line_end(row - 1)}; }

    /**
    \brief Returns a line of characters. The line is extracted from the location
    parameter.

    \param[in] location The location containing the row number.

    \returns A vector of all characters on the row in location.
    */
    string get_line(const Location& location) const { return get_line(location.row); }
    /**
    \brief Get the whole input as a string.

    \returns A string containing all read characters.
    */
    string get_all() const { return transform<vector<char>, string>(_charBuffer); }
    /**
    \brief Returns the location after n-character rollback from a previous
    location.

    \param[in] location The location from which unget is called.
    \param[in] rollback The number of characters by which the rollback is made.

    \returns Location after rollback.

    If the rollback is more characters than has been read, the location of the
    first character in the buffer is returned.
    */
    Location unget(const Location& location, size_t rollback = 1) const noexcept {
      const auto begin = _lineStartBuffer.begin();
      // index of rolled back character
      size_t index = character(location) - rollback - _charBuffer.begin();
      // underflow check, return first location
      if (index > (size_t(character(location) - _charBuffer.begin()))) {
        return Location{location.fileName};
      }
      // find first line after the current
      auto it = std::upper_bound(_lineStartBuffer.begin(), _lineStartBuffer.end(), index);
      --it;
      return {uint64_t(it - begin) + 1, index - *it + 1, location.fileName};
    }

    /**
    \brief Returns the next location based on the read character.

    \param[in] c The read character.
    \param[in] location Previous location.

    \returns The next location after c has been read.
    */
    Location next_location(int c, const Location& location) const noexcept {
      if (c == eof) {
        return location;
      } else if (c == '\n') {
        return {location.row + 1, 1, location.fileName};
      } else {
        return {location.row, location.col + 1, location.fileName};
      }
    }

   private:
    /**
    \brief Stores all read characters.

    The characters are stored in a 1D vector, but are externally segmented into
    lines.
    */
    vector<char> _charBuffer;
    /**
    \brief Stores indices indicating the starts of lines.

    Is used to segment _charBuffer into lines.
    */
    vector<size_t> _lineStartBuffer;
    /**
    \brief The index of EOF.

    This index is set to maximal size_t value until EOF was appended.
    */
    size_t eofLocation_ = std::numeric_limits<size_t>::max();

    /**
    \brief Returns an iterator to the character in location l.

    \param[in] l The location of the retreived character.

    \returns A constant iterator to the character.	If the character is not
    in the line, the returned iterator is larger than the line's line_end()
    iterator.
    */
    vector<char>::const_iterator character(const Location& l) const noexcept {
      return line_begin(line(l)) + col(l);
    }

    /**
    \brief Transforms Location row to line index.

    \param[in] l The location object used as the source of the returned line
    number.

    \returns The line number starting from 0.
    */
    size_t line(const Location& l) const noexcept { return l.row - 1; }
    /**
    \brief Transforms Location col to column index.

    \param[in] l The location object used as the source of the returned line
    number.

    \returns The column number starting from 0.
    */
    size_t col(const Location& l) const noexcept { return l.col - 1; }

    /**
    \brief Get a constant iterator to the first character on a line.

    \param[in] line The begin of this line number is returned.

    \returns A const iterator to the first character on the line.
    */
    vector<char>::const_iterator line_begin(size_t line) const noexcept {
      // line not read yet, begins at end
      if (line >= _lineStartBuffer.size()) {
        return _charBuffer.cend();
      }
      // already read line
      return _charBuffer.cbegin() + _lineStartBuffer[line];
    }

    /**
    \brief Get a constant iterator to the character beyond a '\n' character on a
    line.

    \param[in] line The end of this line is returned.

    \returns A const iterator to the first character beyond the line.
    */
    vector<char>::const_iterator line_end(size_t line) const noexcept {
      // no next line info, ends at end of buffer
      if (line == std::numeric_limits<size_t>::max() || line + 1 >= _lineStartBuffer.size()) {
        return _charBuffer.cend();
      }
      // ends at beginning of next line
      return _charBuffer.cbegin() + _lineStartBuffer[line + 1];
    }
  };  // class InputBuffer
  /**
  \brief Input stream address. Is mutable via change_stream().
  */
  std::istream* _is = nullptr;
  /**
  \brief The name of the string stored in _is.
  */
  string _streamName;
  /**
  \brief The current read position location.
  */
  Location _currentLocation{_streamName};
  /**
  \brief The input buffer object. Stores all read characters.
  */
  InputBuffer _inputBuffer;
};

}  // namespace ctf

#endif
/*** End of file input_reader.cpp ***/