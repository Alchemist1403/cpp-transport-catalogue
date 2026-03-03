#pragma once

#include "json.h"
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>

namespace json {

class Builder {
private:
    class BaseContext;
    class DictContext;
    class DictValueContext;
    class ArrayContext;

    class BaseContext {
    protected:
        Builder& builder_;
        explicit BaseContext(Builder& builder) : builder_(builder) {}

    public:
        BaseContext& Key(const std::string& key) {
            builder_.DoKey(key);
            return *this;
        }
        
        BaseContext& Value(const Node::Value& value) {
            builder_.DoValue(value);
            return *this;
        }
        
        BaseContext& StartDict() {
            builder_.DoStartContainer(Node(Dict{}));
            return *this;
        }
        
        BaseContext& StartArray() {
            builder_.DoStartContainer(Node(Array{}));
            return *this;
        }
        
        BaseContext& EndDict() {
            builder_.DoEndDict();
            return *this;
        }
        
        BaseContext& EndArray() {
            builder_.DoEndArray();
            return *this;
        }
        
        Node Build() {
            return builder_.Build();
        }

    };

    class DictContext: public BaseContext {
    public:
        explicit DictContext(Builder& builder) : BaseContext(builder) {}
        
        DictValueContext Key(const std::string& key);
        Builder& EndDict();

        BaseContext& Value(const Node::Value& value) = delete;
        BaseContext& StartDict() = delete;
        BaseContext& StartArray() = delete;
        BaseContext& EndArray() = delete;
    };


    class DictValueContext: public BaseContext {
    public:
        explicit DictValueContext(Builder& builder) : BaseContext(builder) {}
        
        DictContext Value(const Node::Value& value);
        DictContext StartDict();
        ArrayContext StartArray();

        BaseContext& Key(const std::string&) = delete;
        BaseContext& EndDict() = delete;
        BaseContext& EndArray() = delete;
    };


    class ArrayContext: public BaseContext {
    public:
        explicit ArrayContext(Builder& builder) : BaseContext(builder) {}
        
        ArrayContext Value(const Node::Value& value);
        DictContext StartDict();
        ArrayContext StartArray();
        Builder& EndArray();
        
        BaseContext& Key(const std::string&) = delete;
        BaseContext& EndDict() = delete;
    };


public:
    Node Build();
    DictContext StartDict();
    ArrayContext StartArray();
    Builder& Value(const Node::Value& value);

    Builder& Key(const std::string& key);
    Builder& EndDict();
    Builder& EndArray();
    
protected:
    void DoStartContainer( Node new_node);
    void DoValue(const Node::Value& value);
    void DoKey(const std::string& key);
    void DoEndDict();
    void DoEndArray();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> current_key_;
    bool root_set_ = false;
};

} // namespace json

