#include "json_builder.h"
#include <stdexcept>

namespace json {

DictContext Builder::StartDict() {
    DoStartDict();
    return DictContext(*this);
}

ArrayContext Builder::StartArray() {
    DoStartArray();
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

void Builder::DoStartDict() {
    Node new_node(Dict{});
    
    if (nodes_stack_.empty()) {
        if (root_set_) throw std::logic_error("Root already set");
        root_ = std::move(new_node);
        root_set_ = true;
        nodes_stack_.push_back(&root_);
    } else {
        Node* ctx = nodes_stack_.back();
        if (ctx->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error("StartDict() in dict without Key()");
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
            throw std::logic_error("StartDict() in invalid context");
        }
    }
}

void Builder::DoStartArray() {
    Node new_node(Array{});
    
    if (nodes_stack_.empty()) {
        if (root_set_) throw std::logic_error("Root already set");
        root_ = std::move(new_node);
        root_set_ = true;
        nodes_stack_.push_back(&root_);
    } else {
        Node* ctx = nodes_stack_.back();
        if (ctx->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error("StartArray() in dict without Key()");
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
            throw std::logic_error("StartArray() in invalid context");
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


DictValueContext DictContext::Key(const std::string& key) {
    builder_.DoKey(key);
    return DictValueContext(builder_);
}

Builder& DictContext::EndDict() {
    builder_.DoEndDict();
    return builder_;
}


DictContext DictValueContext::Value(const Node::Value& value) {
    builder_.DoValue(value);
    return DictContext(builder_);
}

DictContext DictValueContext::StartDict() {
    builder_.DoStartDict();
    return DictContext(builder_);
}

ArrayContext DictValueContext::StartArray() {
    builder_.DoStartArray();
    return ArrayContext(builder_);
}


ArrayContext ArrayContext::Value(const Node::Value& value) {
    builder_.DoValue(value);
    return *this;
}

DictContext ArrayContext::StartDict() {
    builder_.DoStartDict();
    return DictContext(builder_);
}

ArrayContext ArrayContext::StartArray() {
    builder_.DoStartArray();
    return *this;
}

Builder& ArrayContext::EndArray() {
    builder_.DoEndArray();
    return builder_;
}

} // namespace json