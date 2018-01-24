/*
 * Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0, as
 * published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an
 * additional permission to link the program and your derivative works
 * with the separately licensed software that they have included with
 * MySQL.
 *
 * Without limiting anything contained in the foregoing, this file,
 * which is part of MySQL Connector/C++, is also subject to the
 * Universal FOSS Exception, version 1.0, a copy of which can be found at
 * http://oss.oracle.com/licenses/universal-foss-exception.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef MYSQLX_RESULT_H
#define MYSQLX_RESULT_H

/**
  @file
  Classes used to access query and command execution results.
*/

#include "common.h"
#include "document.h"
#include "row.h"
#include "collations.h"
#include "detail/result.h"


namespace mysqlx {

using std::ostream;

class Session;
class Schema;
class Collection;
class Result;
class Row;
class RowResult;
class SqlResult;
class DbDoc;
class DocResult;

template <class Res, class Op> class Executable;


namespace internal {

/*
  A wrapper which adds methods common for all result classes.
*/

template <class Base>
class Result_common
  : protected Base
{
  using WarningList = internal::Result_detail::WarningList;

public:

  /// Get the number of warnings stored in the result.

  unsigned getWarningCount() const
  {
    try {
      return Base::get_warning_count();
    }
    CATCH_AND_WRAP
  }

  /// Get a list of warnings stored in the result.

  WarningList getWarnings()
  {
    try {
      return Base::get_warnings();
    }
    CATCH_AND_WRAP
  }

  /// Get the warning at the given, 0-based position.
  // TODO: Change arg type to size_t?

  Warning getWarning(unsigned pos)
  {
    try {
      return Base::get_warning(pos);
    }
    CATCH_AND_WRAP
  }

  // TODO: expose this in the API?
  //using WarningsIterator = Result_detail::iterator;

protected:

  // Wrap base ctors/assginment with catch handlers

  Result_common()
  try
    : Base()
  {}
  CATCH_AND_WRAP

  Result_common(Result_common &&other)
  try
    : Base(std::move(other))
  {}
  CATCH_AND_WRAP

  Result_common& operator=(Result_common &&other)
  try
  {
    Base::operator=(std::move(other));
    return *this;
  }
  CATCH_AND_WRAP

  Result_common(common::Result_init &init)
  try
    : Base(init)
  {}
  CATCH_AND_WRAP

};

}  // internal namespace


/**
  Represents a result of an operation that does not return data.

  A generic result which can be returned by operations which only
  modify data.

  A `Result` instance can store the result of executing an operation:

  ~~~~~~
  Result res = operation.execute();
  ~~~~~~

  Storing another result in a `Result` instance overwrites
  the previous result.

  @ingroup devapi_res
*/

class Result
  : public internal::Result_common<internal::Result_detail>
{

public:

  Result() = default;

  /**
    Get the count of affected items from manipulation statements.
  */

  uint64_t getAffectedItemsCount() const
  {
    try {
      return Result_detail::get_affected_rows();
    } CATCH_AND_WRAP
  }

  /**
    Get the auto-increment value if one was generated by a table insert
    statement.
  */

  uint64_t getAutoIncrementValue() const
  {
    try {
      return Result_detail::get_auto_increment();
    } CATCH_AND_WRAP
  }


  /**
    Return an identifier of a single document added to a collection.
  */

  const GUID& getDocumentId() const
  {
    try {
      return Result_detail::get_document_id();
    } CATCH_AND_WRAP
  }


  /**
    Return a list of identifiers of multiple documents added to a collection.
  */

  DocIdList getDocumentIds() const
  {
    try {
      return Result_detail::get_document_ids();
    } CATCH_AND_WRAP
  }

private:


  Result(common::Result_init &init)
    : Result_common(init)
  {}

  template <class Res, class Op>
  friend class Executable;
  friend Collection;
};



// Row based results
// -----------------

/**
  Types that can be reported in result meta-data.

  @ingroup devapi_res
*/

enum class Type : unsigned short
{
#undef TYPE_ENUM
#define TYPE_ENUM(X,N) X=N,

  RESULT_TYPE_LIST(TYPE_ENUM)
};


/**
  Return name of a given type.

  @ingroup devapi_res
*/

inline
const char* typeName(Type t)
{
#define TYPE_NAME(T,X) case Type::T: return #T;

  switch (t)
  {
    RESULT_TYPE_LIST(TYPE_NAME)
  default:
    THROW("Unknown type");
  }
}

inline
std::ostream& operator<<(std::ostream &out, Type t)
{
  return out << typeName(t);
}


/**
  Provides meta-data for a single result column.

  @ingroup devapi_res
*/

class Column
  : public virtual common::Printable
  , private internal::Column_detail
{
public:

  string getSchemaName()  const  ///< TODO
  {
    try {
      return Column_detail::get_schema_name();
    }
    CATCH_AND_WRAP
  }

  string getTableName()   const  ///< TODO
  {
    try {
      return Column_detail::get_table_name();
    }
    CATCH_AND_WRAP
  }

  string getTableLabel()  const  ///< TODO
  {
    try {
      return Column_detail::get_table_label();
    }
    CATCH_AND_WRAP
  }

  string getColumnName()  const  ///< TODO
  {
    try {
      return Column_detail::get_name();
    }
    CATCH_AND_WRAP
  }

  string getColumnLabel() const  ///< TODO
  {
    try {
      return Column_detail::get_label();
    }
    CATCH_AND_WRAP
  }

  Type getType()   const  ///< TODO
  {
    try {
      return Type(Column_detail::get_type());
    }
    CATCH_AND_WRAP
  }

  /**
    Get column length

    @return The maximum length of data in the column in bytes, as reported
    by the server.

    @note Because the column length is returned as byte length, it could be
    confusing with the multi-byte character sets. For instance, with UTF8MB4
    the length of VARCHAR(100) column is reported as 400 because each character
    is 4 bytes long.
  */

  unsigned long getLength() const
  {
    try {
      return Column_detail::get_length();
    }
    CATCH_AND_WRAP
  }

  unsigned short getFractionalDigits() const  ///< TODO
  {
    try {
      return Column_detail::get_decimals();
    }
    CATCH_AND_WRAP
  }

  bool isNumberSigned() const  ///< TODO
  {
    try {
      return Column_detail::is_signed();
    }
    CATCH_AND_WRAP
  }

  CharacterSet getCharacterSet() const  ///< TODO
  {
    try {
      return Column_detail::get_charset();
    }
    CATCH_AND_WRAP
  }

  /// TODO
  std::string getCharacterSetName() const
  {
    try {
      return characterSetName(getCharacterSet());
    }
    CATCH_AND_WRAP
  }

  const CollationInfo& getCollation() const  ///< TODO
  {
    try {
      return Column_detail::get_collation();
    }
    CATCH_AND_WRAP
  }

  /// TODO
  std::string getCollationName() const
  {
    try {
      return getCollation().getName();
    }
    CATCH_AND_WRAP
  }

  /// TODO
  bool isPadded() const
  {
    try {
      return Column_detail::is_padded();
    }
    CATCH_AND_WRAP
  }

protected:


  using Column_detail::Impl;

  Column(const Impl &impl)
  try
    : Column_detail(impl)
  {}
  CATCH_AND_WRAP

  Column() = default;
  Column(const Column&) = default;
  Column(Column&&) = default;

  Column& operator=(const Column&) = default;

  void print(std::ostream &out) const
  {
    // TODO: not sure if this code will be called by operator<<.
    try {
      Column_detail::print(out);
    }
    CATCH_AND_WRAP
  }

public:

  friend RowResult;
  struct INTERNAL Access;
  friend Access;
};


/*
  Extern declarations for Columns_detail<Column> template specialization
  elements that are defined in result.cc.

  Note: "extern template" works with MSVC but not with GCC.
*/

namespace internal {

template<> PUBLIC_API
void Columns_detail<Column>::init(const Result_detail::Impl&);

}  // internal


extern template PUBLIC_API
void internal::Columns_detail<Column>::init(
  const internal::Result_detail::Impl &impl
);


class Columns
  : private internal::Columns_detail<Column>
{
public:

  using Columns_detail::operator[];

  using Columns_detail::iterator;

  using Columns_detail::begin;
  using Columns_detail::end;

private:

  using Columns_detail::init;

  // note: Required by Row_result_detail

  Columns() = default;
  Columns(Columns&&) = default;
  Columns& operator=(Columns&&) = default;

  friend internal::Row_result_detail<Columns>;
};


/*
  Extern declarations for Row_result_detail<Columns> template specialization
  elements that are defined in result.cc.
*/

namespace internal {

template<> PUBLIC_API
bool Row_result_detail<Columns>::iterator_next();

template<> PUBLIC_API
col_count_t Row_result_detail<Columns>::col_count() const;

template<> PUBLIC_API
Row_result_detail<Columns>::Row_result_detail(
  common::Result_init &init
);

template<> PUBLIC_API
auto Row_result_detail<Columns>::get_column(col_count_t pos) const
-> const Column&;

template<> PUBLIC_API
auto internal::Row_result_detail<Columns>::get_columns() const
-> const Columns&;

template<> PUBLIC_API
row_count_t internal::Row_result_detail<Columns>::row_count();

} // internal


/**
  %Result of an operation that returns rows.

  A `RowResult` object gives sequential access to the rows contained in
  the result. It is possible to get the rows one-by-one, or fetch and store
  all of them at once. One can iterate over the rows using range loop:
  `for (Row r : result) ...`.

  @ingroup devapi_res
*/

class RowResult
    : public internal::Result_common<internal::Row_result_detail<Columns>>
{
public:

  using Columns = mysqlx::Columns;

  RowResult() = default;


  /// Return the number of fields in each row.

  col_count_t getColumnCount() const
  {
    try {
      return Row_result_detail::col_count();
    }
    CATCH_AND_WRAP
  }

  /// Return `Column` object describing the given column of the result.

  const Column& getColumn(col_count_t pos) const
  {
    try {
      return Row_result_detail::get_column(pos);
    }
    CATCH_AND_WRAP
  }

  /**
    Return meta-data for all result columns.

    TODO: explain ownership
  */

  const Columns& getColumns() const
  {
    try {
      return Row_result_detail::get_columns();
    }
    CATCH_AND_WRAP
  }

  /**
    Return the current row and move to the next one in the sequence.

    If there are no more rows in this result, returns a null `Row` instance.
  */

  Row fetchOne()
  {
    try {
      return Row_result_detail::get_row();
    }
    CATCH_AND_WRAP
  }

  using iterator = RowList::iterator;

  /**
    Return all remaining rows

    %Result of this method can be stored in a container such as
    `std::list<Row>`. Rows that have already been fetched using `fetchOne()` are
    not included in the result of `fetchAll()`.
   */

  RowList fetchAll()
  {
    try {
      return Row_result_detail::get_rows();
    }
    CATCH_AND_WRAP
  }

  /**
    Returns the number of rows contained in the result.

    The method counts only the rows that were not yet fetched and are still
    available in the result.
  */

  row_count_t count()
  {
    try {
      return Row_result_detail::row_count();
    }
    CATCH_AND_WRAP
  }

  /*
   Iterate over rows (range-for support).

   Rows that have been fetched using iterator are not available when
   calling fetchOne() or fetchAll()
  */

  iterator begin()
  {
    try {
      return Row_result_detail::begin();
    }
    CATCH_AND_WRAP
  }

  iterator end() const
  {
    try {
      return Row_result_detail::end();
    }
    CATCH_AND_WRAP
  }


private:

  RowResult(common::Result_init &init)
    : Result_common(init)
  {}

public:

  template <class Res, class Op> friend class Executable;
  friend SqlResult;
  friend DocResult;
};


/**
  %Result of an SQL query or command.

  In general, an SQL query or command can return multiple results (for example,
  a call to a stored procedure). Additionally, each or only some of these
  results can contain row data. A `SqlResult` object gives a sequential access
  to all results of a multi-result. Method `nextResult()` moves to the next
  result in the sequence, if present. Methods of `RowResult` are used to access
  row data of the current result (if it contains data).

  @ingroup devapi_res
*/

class SqlResult
  : public RowResult
{
public:

  SqlResult() = default;

  /**
    Tell if the current result contains row data.

    If this is the case, rows can be accessed using `RowResult` interface.
    Otherwise calling `RowResult` methods throws an error.
  */

  bool hasData() const
  {
    try {
      return Result_detail::has_data();
    }
    CATCH_AND_WRAP
  }


  /**
    Move to the next result, if there is one.

    Returns true if the next result is available, false if there are no more
    results in the reply. Calling `nextResult()` discards the current result.
    If it has any rows that has not yet been fetched, these rows are also
    discarded.
  */

  bool nextResult()
  {
    try {
      return Result_detail::next_result();
    }
    CATCH_AND_WRAP
  }


  /**
    Get the count of affected items from data manipulation statements.
  */

  uint64_t getAffectedRowsCount()
  {
    try {
      return Result_detail::get_affected_rows();
    }
    CATCH_AND_WRAP
  }


  /**
    Get the auto-increment value if one was generated by a table insert
    statement.
  */

  uint64_t getAutoIncrementValue()
  {
    try {
      return Result_detail::get_auto_increment();
    }
    CATCH_AND_WRAP
  }

private:

  SqlResult(common::Result_init &init)
    : RowResult(init)
  {}

  template <class Res, class Op>
  friend class Executable;
};


// Document based results
// ----------------------


/**
  %Result of an operation that returns documents.

  A `DocResult` object gives sequential access to the documents contained in
  the result. It is possible to get the documents one-by-one, or fetch and store
  all of them at once. One can iterate over the documents using range loop:
  `for (DbDoc d : result) ...`.

  @ingroup devapi_res
*/

class DocResult
  : public internal::Result_common<internal::Doc_result_detail>
{

public:

  DocResult() = default;

  /**
    Return the current document and move to the next one in the sequence.

    If there are no more documents in this result, returns a null document.
  */

  DbDoc fetchOne()
  {
    try {
      return Doc_result_detail::get_doc();
    }
    CATCH_AND_WRAP
  }

  /**
    Return all remaining documents.

    %Result of this method can be stored in a container such as
    `std::list<DbDoc>`. Documents that have already been fetched using
    `fetchOne()` are not included in the result of `fetchAll()`.
   */

  DocList fetchAll()
  {
    try {
      return Doc_result_detail::get_docs();
    }
    CATCH_AND_WRAP
  }

  /**
    Returns the number of documents contained in the result.

    The method counts only the documents that were not yet fetched and are still
    available in the result.
  */

  uint64_t count()
  {
    try {
      return Doc_result_detail::count();
    }
    CATCH_AND_WRAP
  }

  /*
   Iterate over documents (range-for support).

   Documents that have been fetched using iterator are not available when
   calling fetchOne() or fetchAll()
  */

  using iterator = Doc_result_detail::iterator;

  iterator begin()
  {
    try {
      return Doc_result_detail::begin();
    }
    CATCH_AND_WRAP
  }

  iterator end() const
  {
    try {
      return Doc_result_detail::end();
    }
    CATCH_AND_WRAP
  }


private:

  DocResult(common::Result_init &init)
    : Result_common(init)
  {}

  friend DbDoc;
  template <class Res,class Op>
  friend class Executable;
};


}  // mysqlx

#endif
