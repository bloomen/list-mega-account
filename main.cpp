#include <iostream>
#include <future>

#include <megaapi.h>

class RequestListener : public mega::MegaRequestListener {
public:

    // Make the promise ready for the next request
    std::future<bool> prepare() {
        mPromise = std::promise<bool>{};
        return mPromise.get_future();
    }

    // Called on the SDK thread whenever a request finishes
    void onRequestFinish(mega::MegaApi* megaApi, mega::MegaRequest* request, mega::MegaError* e) override {
        switch (request->getType()) {
            case mega::MegaRequest::TYPE_LOGIN: {
                if (e->getErrorCode() != mega::MegaError::API_OK) {
                    std::cerr << "Login failure: " << e->getErrorCode() << std::endl;
                    mPromise.set_value(false);
                } else {
                    std::cout << "Login success" << std::endl;
                    megaApi->fetchNodes(this);
                }
                break;
            }
            case mega::MegaRequest::TYPE_FETCH_NODES: {
                if (e->getErrorCode() != mega::MegaError::API_OK) {
                    std::cerr << "Fetchnodes failure: " << e->getErrorCode() << std::endl;
                    mPromise.set_value(false);
                } else {
                    std::cout << "Fetchnodes success" << std::endl;
                    mPromise.set_value(true);
                }
                break;
            }
            case mega::MegaRequest::TYPE_LOGOUT: {
                if (e->getErrorCode() != mega::MegaError::API_OK) {
                    std::cerr << "Logout failure: " << e->getErrorCode() << std::endl;
                    mPromise.set_value(false);
                } else {
                    std::cout << "Logout success" << std::endl;
                    mPromise.set_value(true);
                }
                break;
            }
        }
    }

private:
    std::promise<bool> mPromise;
};

// Prints all nodes recursively starting at `node`
void printNodes(mega::MegaApi& megaApi, mega::MegaNode& node) {
    const auto isFile = node.getType() == mega::MegaNode::TYPE_FILE;
    std::unique_ptr<char[]> path{megaApi.getNodePath(&node)};
    std::cout << (isFile ? "F " : "D ") << path.get() << std::endl;
    if (isFile) {
        return;
    }
    std::unique_ptr<mega::MegaNodeList> children{megaApi.getChildren(&node)};
    if (children) {
        // Recursively go through children
        for (int i = 0; i < children->size(); ++i) {
            printNodes(megaApi, *children->get(i));
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " appkey username password" << std::endl;
        return EXIT_FAILURE;
    }
    mega::MegaApi megaApi{argv[1]}; // AppKey can be retrieved from: https://mega.nz/#sdk
    RequestListener listener;

    std::future<bool> loginFuture = listener.prepare();
    megaApi.login(argv[2], argv[3], &listener); // Logging into our account. Fetches remote nodes
    if (!loginFuture.get()) {
        return EXIT_FAILURE;
    }

    std::unique_ptr<mega::MegaNode> rootNode{megaApi.getRootNode()}; // Get the root remote node
    printNodes(megaApi, *rootNode); // Printing all nodes

    std::future<bool> logoutFuture = listener.prepare();
    megaApi.logout(&listener); // Logout from our account
    if (!logoutFuture.get()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
