#define BOOST_TEST_MODULE CDequeTest

#include <boost/test/unit_test.hpp>
#include <boost/container/string.hpp>

#include "tupl/pvt/ArrayDeque.hpp"

using tupl::pvt::ArrayDeque;

BOOST_AUTO_TEST_CASE(ArrayDequeBasicTest) {

	ArrayDeque<int,4> cursor;

	BOOST_CHECK(cursor.empty());
	BOOST_CHECK(!cursor.full());

	cursor.emplaceBack(10);
	BOOST_CHECK(!cursor.empty());
	BOOST_CHECK(cursor.back() == 10);
	BOOST_CHECK(cursor.front() == 10);
	BOOST_CHECK(cursor.size() == 1);
	BOOST_CHECK(!cursor.full());

	cursor.emplaceFront(100);
	BOOST_CHECK(!cursor.empty());
	BOOST_CHECK(cursor.back() == 10);
	BOOST_CHECK(cursor.front() == 100);
	BOOST_CHECK(cursor.size() == 2);
	BOOST_CHECK(!cursor.full());

	cursor.emplaceBack(20);
	BOOST_CHECK(!cursor.empty());
	BOOST_CHECK(cursor.back() == 20);
	BOOST_CHECK(cursor.front() == 100);
	BOOST_CHECK(cursor.size() == 3);
	BOOST_CHECK(!cursor.full());

	cursor.emplaceFront(200);
	BOOST_CHECK(!cursor.empty());
	BOOST_CHECK(cursor.back() == 20);
	BOOST_CHECK(cursor.front() == 200);
	BOOST_CHECK(cursor.size() == 4);
	BOOST_CHECK(cursor.full());

	cursor.popFront();
	BOOST_CHECK(!cursor.empty());
	BOOST_CHECK(cursor.back() == 20);
	BOOST_CHECK(cursor.front() == 100);
	BOOST_CHECK(cursor.size() == 3);
	BOOST_CHECK(!cursor.full());

	cursor.popBack();
	BOOST_CHECK(!cursor.empty());
	BOOST_CHECK(cursor.back() == 10);
	BOOST_CHECK(cursor.front() == 100);
	BOOST_CHECK(cursor.size() == 2);
	BOOST_CHECK(!cursor.full());

	cursor.popBack();
	BOOST_CHECK(!cursor.empty());
	BOOST_CHECK(cursor.back() == 100);
	BOOST_CHECK(cursor.front() == 100);
	BOOST_CHECK(cursor.size() == 1);
	BOOST_CHECK(!cursor.full());

	cursor.popFront();
	BOOST_CHECK(cursor.empty());
	BOOST_CHECK(!cursor.full());

}