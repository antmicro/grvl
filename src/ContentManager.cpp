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

#include "ContentManager.h"
#include "Image.h"

namespace grvl {

    ContentManager::~ContentManager()
    {
    }

    void ContentManager::UpdateContent(string& name, ImageContent* ic)
    {
        BindingsToRemove bindingsToErase;
        ImageBindingMap::iterator it = MissingContent.begin();
        while(it != MissingContent.end()) {
            if(it->second.length() == name.length() && strcmp(it->second.c_str(), name.c_str()) == 0) {
                it->first->ReplaceImageContent(ic);
                bindingsToErase.push_back(it);
            }
            it++;
        }

        // Solves STL incompatibility
        while(!bindingsToErase.empty()) {
            MissingContent.erase(bindingsToErase.back());
            bindingsToErase.pop_back();
        }
    }

    void ContentManager::AddInternalImageContent(string& name, ImageContent* ic)
    {
        InternalImageContainer[name] = ic;
    }

    void ContentManager::AddExternalImageContent(string& name, ImageContent* ic)
    {
        ExternalImageContainer[name] = ic;

        // Check if request for the content is pending
        for(PendingRequestsVector::iterator it = PendingRequests.end() - 1; it >= PendingRequests.begin(); it--) {
            if(it->length() == name.length() && strcmp(it->c_str(), name.c_str()) == 0) {
                UpdateContent(name, ic);

                // Remove request from the list
                PendingRequests.erase(it);
                break;
            }
        }
    }

    bool ContentManager::TryToFindInInternalContent(const string& contentName, Image* image)
    {
        ImageContentMap::const_iterator searchLocalContainer = InternalImageContainer.find(contentName);
        if(searchLocalContainer != InternalImageContainer.end()) {
            image->ReplaceImageContent(searchLocalContainer->second);
            return true;
        }
        return false;
    }
    bool ContentManager::TryToFindInExternalContent(const string& contentName, Image* image)
    {
        ImageContentMap::const_iterator searchExternalContainer = ExternalImageContainer.find(contentName);
        if(searchExternalContainer != ExternalImageContainer.end()) {
            image->ReplaceImageContent(searchExternalContainer->second);
            return true;
        }
        return false;
    }

    void ContentManager::BindImageContentToImage(const string& contentName, Image* image)
    {
        bool contentBinded = false;

        // Check internal image content container
        contentBinded = TryToFindInInternalContent(contentName, image);

        // Check external image container
        if(!contentBinded) {
            contentBinded = TryToFindInExternalContent(contentName, image);
        }

        // If image content is not available add request
        // Check if image is already registered
        ImageBindingMap::iterator it = MissingContent.find(image);
        if(contentBinded && it != MissingContent.end()) {
            MissingContent.erase(it); // remove pending request
        } else if(!contentBinded && it == MissingContent.end()) {
            MissingContent[image] = contentName;
        }

        if(!contentBinded) {
            image->bindingRegistered = true;
        }
    }

    void ContentManager::RequestBinding(Image* image)
    {
        string requestedContentName = MissingContent[image];
        if(requestedContentName.length() == 0) {
            return;
        }

        // Check if not pending already
        for(PendingRequestsVector::iterator it = PendingRequests.begin(); it < PendingRequests.end(); ++it) {
            if(strcmp(it->c_str(), requestedContentName.c_str()) == 0) {
                image->imageContentRequested = true;
                return;
            }
        }

        PendingRequests.push_back(requestedContentName);
        image->imageContentRequested = true;

        // Register the request
        if(RequestContentCallback) {
            RequestContentCallback(requestedContentName);
        }
    }

    void ContentManager::CancelRequest(Image* image)
    {
        string requestedContentName = MissingContent[image];

        if(requestedContentName.length() == 0) {
            return;
        }

        // Check if not pending already
        int numberOfImagesWaitingForBinding = 0;
        PendingRequestsVector::iterator requestToErase;
        for(PendingRequestsVector::iterator it = PendingRequests.begin(); it < PendingRequests.end(); ++it) {
            if(strcmp(it->c_str(), requestedContentName.c_str()) == 0) {
                numberOfImagesWaitingForBinding += 1;
                requestToErase = it;
            }
        }

        if(numberOfImagesWaitingForBinding == 1) {
            CancelRequestCallback(requestedContentName);
            PendingRequests.erase(requestToErase);
            image->imageContentRequested = false;
        }
    }

    void ContentManager::SetRequestCallback(ContentCallback requestCallback)
    {
        if(requestCallback == NULL) {
            return;
        }
        RequestContentCallback = requestCallback;
    }

    void ContentManager::SetCancelRequestCallback(ContentCallback cancelRequestCallback)
    {
        if(cancelRequestCallback == NULL) {
            return;
        }
        CancelRequestCallback = cancelRequestCallback;
    }

} /* namespace grvl */
