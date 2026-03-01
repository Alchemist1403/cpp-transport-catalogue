#pragma once

#include "json.h"
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>

namespace json {

class Builder;

class DictContext;
class DictValueContext;
class ArrayContext;

class DictContext {
public:
    explicit DictContext(Builder& builder) : builder_(builder) {}
    
    DictValueContext Key(const std::string& key);
    Builder& EndDict();
    
private:
    Builder& builder_;
};

class DictValueContext {
public:
    explicit DictValueContext(Builder& builder) : builder_(builder) {}
    
    DictContext Value(const Node::Value& value);
    DictContext StartDict();
    ArrayContext StartArray();
    
private:
    Builder& builder_;
};

class ArrayContext {
public:
    explicit ArrayContext(Builder& builder) : builder_(builder) {}
    
    ArrayContext Value(const Node::Value& value);
    DictContext StartDict();
    ArrayContext StartArray();
    Builder& EndArray();
    
private:
    Builder& builder_;
};


class Builder {
public:
    DictContext StartDict();
    ArrayContext StartArray();
    Builder& Value(const Node::Value& value);
    Node Build();
    
    Builder& Key(const std::string& key);
    Builder& EndDict();
    Builder& EndArray();
    
private:
    void DoStartDict();
    void DoStartArray();
    void DoValue(const Node::Value& value);
    void DoKey(const std::string& key);
    void DoEndDict();
    void DoEndArray();
    
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> current_key_;
    bool root_set_ = false;
    
    friend class DictContext;
    friend class DictValueContext;
    friend class ArrayContext;
};

} // namespace json