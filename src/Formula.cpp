#include "Formula.h"
using namespace std;

P atomF(const string& n) { auto f = make_shared<F>(); f->k = tA; f->n = n; return f; }
P neg(const P& a) { auto f = make_shared<F>(); f->k = tN; f->l = a; return f; }
P bin(K k, const P& a, const P& b) { auto f = make_shared<F>(); f->k = k; f->l = a; f->r = b; return f; }
bool isBin(K k) { return k >= tAnd; }

string opS(K k) {
    switch (k) {
        case tAnd: return "∧";
        case tOr:  return "∨";
        case tImp: return "→";
        default:   return "↔";
    }
}

string S(const P& f) {
    if (f->k == tA) return f->n;
    if (f->k == tN) return "¬" + (isBin(f->l->k) ? "(" + S(f->l) + ")" : S(f->l));
    auto w = [&](const P& c) {
        bool plain = !isBin(c->k) || ((f->k == tAnd || f->k == tOr) && c->k == f->k);
        return plain ? S(c) : "(" + S(c) + ")";
    };
    return w(f->l) + " " + opS(f->k) + " " + w(f->r);
}
