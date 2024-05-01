#pragma once

#ifndef parametric_hpp
#define parametric_hpp

#include <iostream>
#include <string>
#include "exprtk.hpp"
#include "defintions.hpp"

// standard library-provided function for solving an expression
template <typename T>
T evaluateExpression(std::string sExpression, T variable) {
    typedef exprtk::symbol_table<T> symbol_table_t;
    typedef exprtk::expression<T>   expression_t;
    typedef exprtk::parser<T>       parser_t;

    symbol_table_t symbol_table;
    symbol_table.add_variable("t", variable);
    symbol_table.add_constants();

    expression_t expression;
    expression.register_symbol_table(symbol_table);

    parser_t parser;
    parser.compile(sExpression, expression);

    const T y = expression.value();
    return y;
}

// solve for a singular function in a given interval and step
// returns a vector of the function evaluated at each step
std::vector<double> evalInInterval(std::string expressionsInput, double start, double end, double step) {

    // init stuff
    typedef exprtk::symbol_table<double> symbol_table_t;
    typedef exprtk::expression<double>   expression_t;
    typedef exprtk::parser<double>       parser_t;

    std::vector<double> fTerms;
    double currentInput = start;

    // make symbol table to match variables with provided expression
    symbol_table_t symbol_table;

    // assign values to variables / constants
    symbol_table.add_variable("t", currentInput);
    symbol_table.add_constants();

    // create expression and match values in variables to variables in expression
    expression_t expression;
    expression.register_symbol_table(symbol_table);

    // solve it
    parser_t parser;
    parser.compile(expressionsInput, expression);

    while (currentInput <= end) {
        fTerms.push_back(expression.value());
        currentInput += step;
    }

    return fTerms;
}

// calculates explicit values of x, y, and z, as provided functions of t
// returns a vector with 3 subvectors containing the x, y, and z at each step
// i.e. { {x1, x2, x3, ... }, {y1, y2, y3, ... }, {z1, z2, z3, ... } 
vecXYZ parametric3DWithTInterval(std::string expressions[3], double start, double end, double step) {

    vecXYZ parametricTerms;

    for (int i = 0; i < 3; i++) {
        std::vector<double> vcoordinateTerms = evalInInterval(expressions[i], start, end, step);
        parametricTerms.push_back(vcoordinateTerms);
    }

    return parametricTerms;
}

// calculates explicit values of x, y, and z, as provided functions of t
// returns a vector with grouped values of x, y, and z until one value exceeds the min or max
vecXYZ parametric3DWithMinMaxXYZ(std::string expressions[3], double minX, double minY, double minZ, double maxX, double maxY, double maxZ, double step) {

    std::cout << "solving eqn now" << std::endl;
    vecXYZ parametricTerms = { {}, {}, {} };
    double t = 0.0f;
    double cX = 0.0f;
    double cY = 0.0f;
    double cZ = 0.0f;
    bool atLeastOneDimInBounds = true;
    bool allDimInBounds = true;
    bool anyInfs = true;
    bool xOk, yOk, zOk;
    // infinity is the exception
    while ((atLeastOneDimInBounds || anyInfs) && t < 10.0f) {
        cX = evaluateExpression(expressions[0], t);
        cY = evaluateExpression(expressions[1], t);
        cZ = evaluateExpression(expressions[2], t);

        xOk = (cX > minX && cX < maxX); yOk = (cY > minY && cY < maxY); zOk = (cZ > minZ && cZ < maxZ);
        atLeastOneDimInBounds = xOk || yOk || zOk;
        allDimInBounds = xOk && yOk && zOk;
        anyInfs = isinf(cX) || isinf(cY) || isinf(cZ);

        if (allDimInBounds) {
            parametricTerms[0].push_back(cX);
            parametricTerms[1].push_back(cY);
            parametricTerms[2].push_back(cZ);
        }
        t += step;
    }
    std::cout << "solved" << std::endl;
         
    return parametricTerms;
}

// either provides an exact or a darn close f'(t) given f(t) and a t constant
// please make sure expression is differentiable at the variable :)
double pseudoDerivative(std::string expression, double variable) {
    double x2 = variable + 0.01;
    double x1 = variable - 0.01;
    double y2 = evaluateExpression<double>(expression, x2);
    double y1 = evaluateExpression<double>(expression, x1);
    return (y2 - y1) / (x2 - x1);
}

/*
int main(int argc, char** argv) {
    double test = solveEquation<double>("(2x+1)^2", 2.0f);
    std::cout << test << std::endl;

    std::vector<std::string> testEqns = { "2t + 1", "t/2", "t^2" };


    std::vector<std::vector<double>> parametricTest = parametric3D(testEqns, 1, 2, 0.1);
    std::cout << "Values computed" << std::endl;

    for (auto i : parametricTest) {
        for (auto j : i) {
            std::cout << j << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
*/

#endif