// MinIO C++ Library for Amazon S3 Compatible Cloud Storage
// Copyright 2022 MinIO, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "client.h"

int main( int argc, char* argv[] )
{
    std::string host = "test.liza-notes.pl";
    minio::s3::BaseUrl base_url( host, false );
    base_url.port = 9000;

    minio::creds::StaticProvider provider(
        "debil",
        "Robocze1234" );//*/

    // Create S3 client.
    //minio::s3::Client client( base_url, &provider );
    minio::s3::Client client( base_url, &provider );

    std::string bucket_name = "kupa";

    // Upload '/home/user/Photos/asiaphotos.zip' as object name to bucket.
    minio::s3::UploadObjectArgs args;
    args.bucket = bucket_name;
    args.object = "asd/photos-2026.txt";
    args.filename = "/home/mezdej/dupa";

    minio::s3::UploadObjectResponse resp = client.UploadObject( args );
    if( !resp )
    {
        std::cout << "unable to upload object; " << resp.Error() << std::endl;
        return EXIT_FAILURE;
    }
    std::time_t result = std::time( nullptr );
    std::cout << "'" << args.filename << "' is successfully uploaded as "
        << "object '" << args.object << "' to bucket '" << args.bucket << "'."
        << " " << std::asctime( std::localtime( &result ) ) << std::endl;

    return EXIT_SUCCESS;
}
