#include "json_builder.h"
#include <stdexcept>

namespace json {

Builder::DictContext Builder::StartDict() {
    DoStartContainer(Node(Dict{}));
    return DictContext(*this);
}

Builder::ArrayContext Builder::StartArray() {
    DoStartContainer(Node(Array{}));
    return ArrayContext(*this);
}

Builder& Builder::Value(const Node::Value& value) {
    DoValue(value);
    return *this;
}

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Unfinished JSON structure");
    }
    if (!root_set_) {
        throw std::logic_error("Build() called without setting root");
    }
    return root_;
}

Builder& Builder::Key(const std::string& key) {
    DoKey(key);
    return *this;
}

Builder& Builder::EndDict() {
    DoEndDict();
    return *this;
}

Builder& Builder::EndArray() {
    DoEndArray();
    return *this;
}


void Builder::DoStartContainer(Node new_node) {
    if (nodes_stack_.empty()) {
        if (root_set_) throw std::logic_error("Root already set");
        root_ = std::move(new_node);
        root_set_ = true;
        nodes_stack_.push_back(&root_);
    } else {
        Node* ctx = nodes_stack_.back();
        if (ctx->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error("Container in dict without Key()");
            }
            auto& dict = const_cast<Dict&>(ctx->AsDict());
            dict[*current_key_] = std::move(new_node);
            nodes_stack_.push_back(&dict.at(*current_key_));
            current_key_.reset();
        } else if (ctx->IsArray()) {
            auto& arr = const_cast<Array&>(ctx->AsArray());
            arr.emplace_back(std::move(new_node));
            nodes_stack_.push_back(&arr.back());
        } else {
            throw std::logic_error("Container in invalid context");
        }
    }
}


void Builder::DoValue(const Node::Value& value) {
    Node node(value);
    
    if (nodes_stack_.empty()) {
        if (root_set_) throw std::logic_error("Root already set");
        root_ = std::move(node);
        root_set_ = true;
    } else {
        Node* ctx = nodes_stack_.back();
        if (ctx->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error("Value() in dict without Key()");
            }
            auto& dict = const_cast<Dict&>(ctx->AsDict());
            dict[*current_key_] = std::move(node);
            current_key_.reset();
        } else if (ctx->IsArray()) {
            auto& arr = const_cast<Array&>(ctx->AsArray());
            arr.emplace_back(std::move(node));
        } else {
            throw std::logic_error("Value() in invalid context");
        }
    }
}


void Builder::DoKey(const std::string& key) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Key() called outside dictionary");
    }
    Node* ctx = nodes_stack_.back();
    if (!ctx->IsDict()) {
        throw std::logic_error("Key() called outside dictionary");
    }
    if (current_key_.has_value()) {
        throw std::logic_error("Key() called twice without Value/StartDict/StartArray");
    }
    current_key_ = key;
}


void Builder::DoEndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("EndDict() without matching StartDict()");
    }
    nodes_stack_.pop_back();
}

void Builder::DoEndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("EndArray() without matching StartArray()");
    }
    nodes_stack_.pop_back();
}


// DictContext

Builder::DictValueContext Builder::DictContext::Key(const std::string& key) {
    builder_.DoKey(key);
    return DictValueContext(builder_);
}

Builder& Builder::DictContext::EndDict() {
    builder_.DoEndDict();
    return builder_;
}


// DictValueContext

Builder::DictContext Builder::DictValueContext::Value(const Node::Value& value) {
    builder_.DoValue(value);
    return DictContext(builder_);
}

Builder::DictContext Builder::DictValueContext::StartDict() {
    builder_.DoStartContainer(Node(Dict{}));
    return DictContext(builder_);
}

Builder::ArrayContext Builder::DictValueContext::StartArray() {
    builder_.DoStartContainer(Node(Array{}));
    return ArrayContext(builder_);
}


// ArrayContext

Builder::ArrayContext Builder::ArrayContext::Value(const Node::Value& value) {
    builder_.DoValue(value);
    return *this;
}

Builder::DictContext Builder::ArrayContext::StartDict() {
    builder_.DoStartContainer(Node(Dict{}));
    return DictContext(builder_);
}

Builder::ArrayContext Builder::ArrayContext::StartArray() {
    builder_.DoStartContainer(Node(Array{}));
    return *this;
}

Builder& Builder::ArrayContext::EndArray() {
    builder_.DoEndArray();
    return builder_;
}

} // namespace json