
#include "lsystem.h"
#include <stack>
#include <memory>
#include <iostream>

std::string LindenmayerSystemDeterministic::expandSymbol(unsigned char const& sym) {
    /*============================================================
        TODO 1.1
        For a given symbol in the sequence, what should it be replaced with after expansion?

        You may find useful:
            map.find: Iterator to an element with key equivalent to key. If no such element is found, past-the-end (see end()) iterator is returned.
            http://en.cppreference.com/w/cpp/container/unordered_map/find
    */


    auto search = rules.find(sym);
        if (search != rules.end()) {
            return search->second;
        }
        else {
            return { char(sym) }; // this constructs string from char
        }

}

std::string LindenmayerSystem::expandOnce(std::string const& symbol_sequence) {
    /*============================================================
        TODO 1.2
        Perform one iteration of grammar expansion on `symbol_sequence`.
        Use the expandSymbol method
    */
    std::string expanded_seq;
    for (int i = 0; i < symbol_sequence.length(); i++) {
        expanded_seq += expandSymbol(symbol_sequence.at(i));
    }

    return expanded_seq;
}

std::string LindenmayerSystem::expand(std::string const& initial, uint32_t num_iters) {
    /*============================================================
        TODO 1.3
        Perform `num_iters` iterations of grammar expansion (use expandOnce)
    */

    std::string seq = initial;
    int i = 0

        while (i != num_iters) {
            seq = expandOnce(seq)
                i++;
        }
    return seq;
}

std::vector<Segment> LindenmayerSystem::draw(std::string const& symbols) {
    std::vector<Segment> lines; // this is already initialized as empty vector
    /*============================================================
        TODO 2
        Build line segments according to the sequence of symbols
        The initial position is (0, 0) and the initial direction is "up" (0, 1)
        Segment is std::pair<vec2, vec2>

        you can create a Segment as {p1, p2}, where p1 and p2 are vec2, e.g.:
        lines.push_back({p1, p2});

        You may also find std::stack useful:

        https://en.cppreference.com/w/cpp/container/stack

        There also is a mat2 class in utils/vec.* you may find useful for
        implementing rotations.
    */

    //============================================================

    vec2 current_position = vec2(0, 0);
    vec2 new_position = vec2(0, 0);
    float angle = 0;
    std::stack<vec2> saved_positions; //initialise a stack with all "stored" positions
    cout << "entered function draw" << endl;

    //todo iterate over all characters in "symbols"
    for (char symbol : symbols) {
        cout << "entered for loop in function draw" << endl;

        //switch case for characters
        switch (symbol) {
        case '+':
            //+ rotate counter clockwise
            angle = angle + rotation_angle_deg;
            break;
        case '-':
            //- roatate clockwise
            angle = angle - rotation_angle_deg;
            break;
        case '[':
            //[ add position to stack
            saved_positions.push(current_position);
            break;
        case ']':
            //] get last position on stack and remove it from stack
            current_position = saved_positions.top();
            saved_position.pop();
            break;
        case 'F':
            //F draw line forward
            new_position = vec2(current_position.x + cos(angle), (current_position.y + sin(angle)));
            lines.push_back({ current_position, new_position });
            current_position = new_position;
            break;
        default:
            cout << "Invalid character" << endl;
        }
    }

    return lines;
}

std::string LindenmayerSystemStochastic::expandSymbol(unsigned char const& sym) {
    /*============================================================
        TODO 4
        For a given symbol in the sequence, what should it be replaced with after expansion?
        (stochastic case)

        Use dice.roll() to get a random number between 0 and 1
    */
    std::string expanded_seq;
    double random = dice.roll()*2;
    if (random < 1) {
        expanded_seq = "F+F";
    }
    else {
        expanded_seq = "F-F";
    }
    return {expanded_seq};
}

void LindenmayerSystemDeterministic::addRuleDeterministic(unsigned char sym, std::string const& expansion) {
    rules[sym] = expansion;
}

void LindenmayerSystemStochastic::addRuleStochastic(unsigned char sym, std::vector<StochasticRule> expansions_with_ps) {
    rules[sym] = expansions_with_ps;
}
