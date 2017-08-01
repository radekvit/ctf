#ifndef CTF_LINE_BUFFER_H_
#define CTF_LINE_BUFFER_H_

#include <algorithm>
#include <limits>
#include "base.hpp"

namespace ctf {

/**
\brief Reads characters from a stream and stores them in a line buffer.

Offers read operations, unget operations, location buffering etc.
Manages read characters and offers character and line retreival.
*/
class InputReader {
 protected:
  /**
  \brief Buffer class with Location-based indexing.

  Characters are exposed as immutable; only appending characters is possible.
  */
  class InputBuffer {
    /**
    \brief Stores all read characters.
    */
    vector<char> charBuffer_;
    /**
    \brief Stores indices indicating the starts of lines.
    */
    vector<size_t> lineStartBuffer_;
    /**
    \brief The index of EOF.
    */
    size_t eofLocation_ = std::numeric_limits<size_t>::max();

    /**
    \brief Returns an iterator to the character in location l.
    \return A constant iterator to the character.	If the character is not
    in
    the line, the returned iterator is larger than the line's line_end()
    iterator.
    */
    vector<char>::const_iterator character(const Location &l) const noexcept {
      return line_begin(line(l)) + col(l);
    }

    /**
    \brief Transforms Location row to line index.
    */
    size_t line(const Location &l) const noexcept { return l.row - 1; }
    /**
    \brief Transforms Location col to column index.
    */
    size_t col(const Location &l) const noexcept { return l.col - 1; }

    /**
    \brief Get a constant iterator to the first character on a line.
    */
    vector<char>::const_iterator line_begin(size_t line) const noexcept {
      // line not read yet, begins at end
      if (line >= lineStartBuffer_.size()) {
        return charBuffer_.cend();
      }
      // already read line
      return charBuffer_.cbegin() + lineStartBuffer_[line];
    }

    /**
    \brief Get a constant iterator to the character beyond a '\n' character on a
    line.
    */
    vector<char>::const_iterator line_end(size_t line) const noexcept {
      // no next line info, ends at end of buffer
      if (line + 1 >= lineStartBuffer_.size()) {
        return charBuffer_.cend();
      }
      // ends at beginning of next line
      return charBuffer_.cbegin() + lineStartBuffer_[line + 1];
    }

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
      charBuffer_.clear();
      lineStartBuffer_.clear();
      lineStartBuffer_.push_back(0);
      eofLocation_ = std::numeric_limits<size_t>::max();
    }
    /**
    \brief Appends the character to the end of the buffer.
    */
    Location append(int c, const Location &location) {
      if (c == eof) {
        eofLocation_ = charBuffer_.size();
      }
      charBuffer_.push_back(c);
      if (c == '\n') {
        lineStartBuffer_.push_back(charBuffer_.size());
      }
      return next_location(c, location);
    }

    /**
    \brief Reads a character and moves a location to its next position.
    \return True if a character is present in the buffer or is eof.
    */
    bool get(int &c, Location &location) const noexcept {
      auto it = character(location);
      if (charBuffer_.size() == eofLocation_ && it == charBuffer_.end()) {
        return eof;
      } else if (it >= line_end(line(location))) {
        return false;
      }
      c = static_cast<int>(*it);
      location = next_location(c, location);
      return true;
    }

    /**
    \brief Returns a line of characters.
    */
    vector<char> get_line(size_t row) const {
      return {line_begin(row - 1), line_end(row - 1)};
    }

    /**
    \brief Returns a line of characters. The line is extracted from the location
    parameter.
    */
    vector<char> get_line(const Location &location) const {
      return get_line(location.row);
    }

    /**
    \brief Returns the location after n-character rollback.

    If the rollback is more characters than read, the location of the first
    character in a file is returned.
    */
    Location unget(const Location &location, size_t num = 1) const noexcept {
      size_t index = charBuffer_.size() - num;
      auto it = std::lower_bound(lineStartBuffer_.begin(),
                                 lineStartBuffer_.end(), index);
      if (it == lineStartBuffer_.end()) {
        return {location.fileName};
      } else {
        return {*it + 1, index - *it + 1, location.fileName};
      }
    }

    /**
    \brief Returns the next location based on the read character.
    */
    Location next_location(int c, const Location &location) const noexcept {
      if (c == '\n') {
        return {location.row + 1, 1, location.fileName};
      } else {
        return {location.row, location.col + 1, location.fileName};
      }
    }
  };  // class InputBuffer
  /**
  \brief Input stream address. Is mutable via change_stream().
  */
  std::istream *is_ = nullptr;
  string streamName_;

  Location currentLocation_ = {streamName_};

  InputBuffer inputBuffer_;

 public:
  InputReader() = default;
  InputReader(std::istream &is, const string &streamName = "") {
    set_stream(is, streamName);
  }

  std::istream *stream() const { return is_; }
  const string &stream_name() const noexcept { return streamName_; }

  void set_stream(std::istream &is, const string &streamName = "") {
    is_ = &is;
    streamName_ = streamName;
    inputBuffer_.reset();
    currentLocation_ = {streamName};
  }

  int get() {
    int c = 0;
    // cannot get
    if (!inputBuffer_.get(c, currentLocation_)) {
      c = is_->get();
      currentLocation_ = inputBuffer_.append(c, currentLocation_);
    }
    return c;
  }

  int get(Location &location) {
    location = currentLocation_;
    return get();
  }

  int unget(size_t num = 1) noexcept {
    int c = 0;
    // rollback one more and read
    currentLocation_ = inputBuffer_.unget(num + 1);
    // will always succeed
    inputBuffer_.get(c, currentLocation_);

    return c;
  }

  int unget(Location &location, size_t num = 1) noexcept {
    int c = 0;
    // rollback one more and read
    currentLocation_ = inputBuffer_.unget(num + 1);
    location = currentLocation_;
    // will always succeed
    inputBuffer_.get(c, currentLocation_);

    return c;
  }
};

}  // namespace ctf

#endif