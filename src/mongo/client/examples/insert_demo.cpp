/*    Copyright 2009 10gen Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// It is the responsibility of the mongo client consumer to ensure that any necessary windows
// headers have already been included before including the driver facade headers.
#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

#include "mongo/client/dbclient.h"  // the mongo c++ driver

#include <iostream>

using namespace std;
using namespace mongo;

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "usage: " << argv[0] << " [MONGODB_URI]" << std::endl;
        return EXIT_FAILURE;
    }

    mongo::client::GlobalInstance instance;
    if (!instance.initialized()) {
        std::cout << "failed to initialize the client driver: " << instance.status() << std::endl;
        return EXIT_FAILURE;
    }

    std::string uri = argc == 2 ? argv[1] : "mongodb://localhost:27017";
    std::string errmsg;

    ConnectionString cs = ConnectionString::parse(uri, errmsg);

    if (!cs.isValid()) {
        std::cout << "Error parsing connection string " << uri << ": " << errmsg << std::endl;
        return EXIT_FAILURE;
    }

    boost::scoped_ptr<DBClientBase> conn(cs.connect(errmsg));
    if (!conn) {
        cout << "couldn't connect : " << errmsg << endl;
        return EXIT_FAILURE;
    }

    try {
        BSONObj o = BSON("hello"
                         << "world");

        cout << "dropping collection..." << endl;
        conn->dropCollection("test.foo");

        cout << "inserting..." << endl;

        time_t start = time(0);
        for (unsigned i = 0; i < 100000; i++) {
            conn->insert("test.foo", o);
        }

        // wait until all operations applied
        cout << "getlasterror returns: \"" << conn->getLastError() << '"' << endl;

        time_t done = time(0);
        time_t dt = done - start;
        cout << dt << " seconds " << 100000 / dt << " per second" << endl;
    } catch (DBException& e) {
        cout << "caught DBException " << e.toString() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
