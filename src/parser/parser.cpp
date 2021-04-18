#include "parser.h"
#include <assert.h>
#include <iostream>

void parser::parser::add_rule(const std::string& name, const std::string& rule) {    
    m_nodes.push_back(m_generator.generate(rule));
    m_rule_map[name] = m_nodes.size() - 1;
    m_name_map[m_nodes.size() - 1] = name;
    m_nodes.rbegin()->print();
}

void parser::parser::add_top_rule (const std::string& name, const std::string& rule) {
    add_rule(name, rule);
    m_top_rules.push_back(name);
}

void parser::parser::init() {
    m_initial_sequence.clear();
    
    for (auto& s : m_top_rules) {
        auto id = m_rule_map.at(s);
        auto tmp = node_initial_expansion(m_nodes[id]);
        
        
        for (auto& r : tmp) {
            r.path.insert(r.path.begin(), id);
            r.history.insert(r.history.begin(), {id, true});
            
            m_initial_sequence.push_back(r);
        }        
    }       
    
//     print(m_initial_sequence);
//     std::cout << "" << std::endl;
//     auto b = sequence_increment(m_initial_sequence[0]);
//     print(b);
}

std::vector<parser::parser::parse_sequence> parser::parser::parse_to_sequence(const std::string& input) const
{
    std::vector<parser::parser::parse_sequence> result = m_initial_sequence;

    for (size_t i = 0; i < input.length(); i++) {
        std::cout << "## Step: " << i << std::endl;
        print(result);
        
        
        std::vector<parser::parser::parse_sequence> active;
        std::vector<parser::parser::parse_sequence> final;
        
        for (auto& current : result) {
            if (sequence_accepts(current, input.at(i))) {
                auto next = sequence_increment(current);
                for (auto& s : next) {
                    active.push_back(s);                    
                }
            }
        }
        
        for (auto& current : active) {
            if (current.path.empty() && i != input.length() - 1) {
                auto s0 = add_sequences(current, m_initial_sequence);
                for (auto& s : s0) {
                    final.push_back(s);
                }                
            } else {
                final.push_back(current);
            }
        }
        
        result = final;
    }    
    
    return result;
}


bool parser::parser::sequence_accepts(const parser::parser::parse_sequence& current, const char c) const {
    assert(!current.path.empty());
    const node* n = &m_nodes[current.path[0]];
    
    for (size_t i = 1; i < current.path.size() - 1; i++) {        
        if (n->children.empty()) {
            auto name = n->text.substr(1, n->text.length() - 2);
            n = &m_nodes[m_rule_map.at(name)];
        }
                
        n = &(n->children[current.path[i]]);
    }
    
    if (
        n->children.size() == 3 &&
        n->children[0].children.empty() &&
        n->children[0].text.length() == 1 &&
        n->children[1].children.empty() &&
        n->children[1].text.length() == 1 &&
        n->children[2].children.empty() &&
        n->children[2].text.length() == 1 &&
        n->children[0].text[0] != '\\' &&
        n->children[1].text[0] == '-' &&
        current.path[current.path.size() - 1] == 1
    ) {        
        return n->children[0].text[0] <= c && c <= n->children[2].text[0];
    } else {        
        assert(n->children.empty());        
        
        return 
            current.path[current.path.size() - 1] < n->text.length() && 
            n->text.at(current.path[current.path.size() - 1]) == c;        
    }
}


std::vector<parser::parser::parse_sequence> parser::parser::sequence_increment(const parser::parser::parse_sequence& current) const {
    assert(!current.path.empty());
    const node* n = &m_nodes[current.path[0]];
    
    std::vector<const node*> node_stack;
    std::vector<size_t> index_stack;
    std::vector<std::pair<size_t, bool>> name_stack;
    
    for (size_t i = 1; i < current.path.size() - 1; i++) {        
        if (n->children.empty()) {
            auto name = n->text.substr(1, n->text.length() - 2);
            n = &m_nodes[m_rule_map.at(name)];
            name_stack.push_back({m_rule_map.at(name), true});
        } else {
            name_stack.push_back({0, false});
        }
        
        node_stack.push_back(n); 
        index_stack.push_back(current.path[i]);
        
        n = &(n->children[current.path[i]]);
    }
    
    if (
        n->children.size() == 3 &&
        n->children[0].children.empty() &&
        n->children[0].text.length() == 1 &&
        n->children[1].children.empty() &&
        n->children[1].text.length() == 1 &&
        n->children[2].children.empty() &&
        n->children[2].text.length() == 1 &&
        n->children[0].text[0] != '\\' &&
        n->children[1].text[0] == '-' &&
        current.path[current.path.size() - 1] == 1
    ) {        
//         return n->children[0].text[0] <= c && c <= n->children[2].text[0];
//         node_stack.pop_back();
//         index_stack.pop_back();
//         name_stack.pop_back();
    } else {        
        if (current.path[current.path.size() - 1] + 1 < n->text.length()) {
            auto result = current;
            result.path[result.path.size() - 1]++;
            return {result};
        }      
    }
    parser::parser::parse_sequence next = current;
    next.path.pop_back();
    
    
//     next.history.push_back(m_name_map.at())
    
    std::vector<parser::parser::parse_sequence> saved_sequences;
    
    for (int i = node_stack.size() - 1; i >= 0; i--) {
        if (index_stack[i] + 1 < node_stack[i]->children.size()) {            
            if (
                node_stack[i]->children[index_stack[i] + 1].children.empty() &&
                node_stack[i]->children[index_stack[i] + 1].text.length() == 1 &&
                node_stack[i]->children[index_stack[i] + 1].text[0] == '|'
            ) {
                // do nothing here
            } else if (
                node_stack[i]->children[index_stack[i] + 1].children.empty() &&
                node_stack[i]->children[index_stack[i] + 1].text.length() == 1 &&
                node_stack[i]->children[index_stack[i] + 1].text[0] == '*'
            ) { 
                auto s0 = node_initial_expansion(node_stack[i]->children[index_stack[i]]);
                auto seq = add_sequences(next, s0);    
                for (auto& s : seq) {
                    saved_sequences.push_back(s);
                }
            } else {                
                next.path[next.path.size() - 1]++;
                auto s0 = node_initial_expansion(node_stack[i]->children[index_stack[i] + 1]);
                auto seq = add_sequences(next, s0);    
                for (auto& s : seq) {
                    saved_sequences.push_back(s);
                }    
                
//                 print(s0);
                
//                 std::cout << "asd " << i << std::endl;

                if (
                    node_stack[i]->children[index_stack[i] + 1].children.size() == 2 &&
                    node_stack[i]->children[index_stack[i] + 1].children[1].text.length() > 0 &&
                    node_stack[i]->children[index_stack[i] + 1].children[1].text[0] == '*'
                ) {
                    
                } else {
                    return saved_sequences;
                }                
            }
        }
        
        if (name_stack[i].second) {
            next.history.push_back({name_stack[i].first, false});
        }
        
        next.path.pop_back();
    }
    
    next.path.clear();
    saved_sequences.push_back(next);
    
    return saved_sequences;
}


std::vector<parser::parser::parse_sequence> parser::parser::node_initial_expansion(const node& n) const {       
    if (n.children.empty()) {
        if (
            n.text.length() > 1 &&
            n.text.at(0) == '[' &&
            n.text.at(n.text.length() - 1) == ']'
        ) {
            auto id = m_rule_map.at(n.text.substr(1, n.text.length() - 2));
            auto s0 = node_initial_expansion(m_nodes[id]);
            
            for (auto& s : s0) {
                s.history.insert(s.history.begin(), {id, true});
            }
            
            return s0;
        }
        
        return {{{0}, {}}};
    } else {
        if (n.children.size() == 3) {
            if (
                n.children[0].children.empty() &&
                n.children[0].text.length() == 1 &&
                n.children[1].children.empty() &&
                n.children[1].text.length() == 1 &&
                n.children[2].children.empty() &&
                n.children[2].text.length() == 1 &&
                n.children[0].text[0] != '\\' &&
                n.children[1].text[0] == '-'
            ) {
                return {{{1}, {}}};
            }
            
            if (
                n.children[0].text[0] != '\\' &&
                n.children[1].children.empty() &&
                n.children[1].text.length() == 1 &&
                n.children[1].text[0] == '|'
            ) {
                
                auto s0 = node_initial_expansion(n.children[0]);
                auto s1 = node_initial_expansion(n.children[2]);
                                
                return merge_sequences(s0, s1);
            }
        }  
        
        auto s0 = node_initial_expansion(n.children[0]);         
    
        return merge_sequences(s0, {});        
    }
}

std::vector<parser::parser::parse_sequence> parser::parser::merge_sequences(const std::vector<parse_sequence>& s0, const std::vector<parse_sequence>& s1) const {
    std::vector<parser::parser::parse_sequence> result;                
    
    for (auto& s : s0) {
        parser::parser::parse_sequence seq;
        seq.path.push_back(0);
        for (auto& p : s.path) {
            seq.path.push_back(p);
        }
        seq.history = s.history;
        
        result.push_back(seq);
    }
    
    for (auto& s : s1) {
        parser::parser::parse_sequence seq;
        seq.path.push_back(2);
        for (auto& p : s.path) {
            seq.path.push_back(p);
        }
        seq.history = s.history;
        
        result.push_back(seq);
    }
    
    return result;
}

std::vector<parser::parser::parse_sequence> parser::parser::add_sequences(const parser::parser::parse_sequence& current, const std::vector<parse_sequence>& s0) const
{
    std::vector<parser::parser::parse_sequence> result;
    
    for (auto& s : s0) {
        parse_sequence seq = current;
        
        for (auto& h : s.history) {
            seq.history.push_back(h);
        }
                
        for (auto& p : s.path) {
            seq.path.push_back(p);
        }        
        
        result.push_back(seq);
    }    
    
    return result;
}


void parser::parser::print(const std::vector<parse_sequence>& seqvec) const {
    for (auto& seq : seqvec) {
        std::cout << "### Sequence ###" << std::endl;
        std::cout << "# History := ";
        for (auto& h : seq.history) {
            if (h.second) {
                std::cout << "<" << m_name_map.at(h.first) << ">";
            } else {
                std::cout << "</" << m_name_map.at(h.first) << ">";                
            }
        }
        std::cout << std::endl;
        std::cout << "# Path := ";
        for (auto& p : seq.path) {
            std::cout << p << " ";
        }
        std::cout << std::endl;
    }
}
