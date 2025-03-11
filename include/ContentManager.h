// Copyright 2014-2024 Antmicro <antmicro.com>
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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef GRVL_CONTENTMANAGER_H_
#define GRVL_CONTENTMANAGER_H_

#include "stl.h"

namespace grvl {
    class Image;
    class ImageContent;

    /// Represents manager for shared resources, e.g., image contents.
    class ContentManager {
    public:
        ContentManager()
            : RequestContentCallback(NULL)
            , CancelRequestCallback(NULL)
        {
        }

        virtual ~ContentManager() = default;

        typedef map<string, ImageContent*> ImageContentMap;
        typedef map<Image*, string> ImageBindingMap;
        typedef ImageBindingMap::iterator ImageBindingIterator;
        typedef vector<ImageBindingIterator> BindingsToRemove;
        typedef vector<string> PendingRequestsVector;

        typedef void (*ContentCallback)(const string contentName);

        void AddInternalImageContent(string& name, ImageContent* ic);
        void AddExternalImageContent(string& name, ImageContent* ic);
        void BindImageContentToImage(const string& contentName, Image* image);

        bool TryToFindInInternalContent(const string& contentName, Image* image);
        bool TryToFindInExternalContent(const string& contentName, Image* image);

        void RequestBinding(Image* image);
        void CancelRequest(Image* image);

        void SetRequestCallback(ContentCallback requestCallback);
        void SetCancelRequestCallback(ContentCallback cancelRequestCallback);

    private:
        ImageContentMap InternalImageContainer; // Can not be deleted
        ImageContentMap ExternalImageContainer; // Can be deleted
        ImageBindingMap MissingContent;
        PendingRequestsVector PendingRequests;
        ContentCallback RequestContentCallback, CancelRequestCallback;

        void UpdateContent(string& name, ImageContent* ic);
    };

} /* namespace grvl */

#endif /* GRVL_CONTENTMANAGER_H_ */
