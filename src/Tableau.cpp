#include "Tableau.h"
#include <algorithm>
#include <iostream>
#include <map>
using namespace std;

bool lit(const P& f, string& n, bool& pos) {
    if (f->k == tA) { n = f->n; pos = true; return true; }
    if (f->k == tN && f->l->k == tA) { n = f->l->n; pos = false; return true; }
    return false;
}
static bool isLit(const P& f) { string n; bool p; return lit(f, n, p); }

// Reglas de expansión. Una sola rama = alfa (∧, ¬∨, ¬→, ¬¬). Varias ramas = beta (∨, →, ↔, ¬∧, ¬↔).
static vector<vector<P>> expansion(const P& f) {
    if (f->k == tAnd) return { { f->l, f->r } };
    if (f->k == tOr)  return { { f->l }, { f->r } };
    if (f->k == tImp) return { { neg(f->l) }, { f->r } };
    if (f->k == tIff) return { { f->l, f->r }, { neg(f->l), neg(f->r) } };
    P a = f->l;   // f es ¬a
    if (a->k == tN)   return { { a->l } };
    if (a->k == tOr)  return { { neg(a->l), neg(a->r) } };
    if (a->k == tImp) return { { a->l, neg(a->r) } };
    if (a->k == tAnd) return { { neg(a->l) }, { neg(a->r) } };
    return { { a->l, neg(a->r) }, { neg(a->l), a->r } };   // ¬↔
}
static string ruleName(const P& f) {
    return f->k == tN ? "¬" + (f->l->k == tN ? string("¬") : opS(f->l->k)) : opS(f->k);
}

Node* Tableau::mk(const P& f, Node* par, int from, const string& tag, const string& rule) {
    pool.push_back(make_unique<Node>());
    Node* n = pool.back().get();
    n->id = (int)pool.size(); n->f = f; n->par = par; n->from = from; n->tag = tag; n->rule = rule;
    if (par) par->kids.push_back(n);
    return n;
}

void Tableau::build(const vector<P>& rules) {
    Node* last = nullptr;
    for (size_t i = 0; i < rules.size(); i++) {
        last = mk(rules[i], last, 0, "R" + to_string(i + 1), "");
        if (!root) root = last;
    }
    vector<Node*> stack{ last };
    while (!stack.empty()) {
        Node* leaf = stack.back(); stack.pop_back();
        vector<Node*> path;
        for (Node* n = leaf; n; n = n->par) path.push_back(n);
        reverse(path.begin(), path.end());

        // ¿hay un átomo y su negación en la rama? -> se cierra
        map<string, Node*> seen; bool closed = false;
        for (Node* n : path) {
            string a; bool pos;
            if (!lit(n->f, a, pos)) continue;
            auto o = seen.find((pos ? "-" : "+") + a);
            if (o != seen.end()) { leaf->end = 1; leaf->clash = S(o->second->f) + " y " + S(n->f); closed = true; break; }
            seen[(pos ? "+" : "-") + a] = n;
        }
        if (closed) { ends.push_back(leaf); continue; }

        // siguiente fórmula por desarrollar: primero las alfa, luego las beta
        set<int> done; for (Node* n : path) if (n->from) done.insert(n->from);
        Node* x = nullptr;
        for (Node* n : path) {
            if (isLit(n->f) || done.count(n->id)) continue;
            if (expansion(n->f).size() == 1) { x = n; break; }
            if (!x) x = n;
        }
        if (!x) { leaf->end = 2; ends.push_back(leaf); continue; }   // solo letras: rama abierta

        string nm = ruleName(x->f);
        vector<Node*> tips;
        for (auto& br : expansion(x->f)) {
            Node* c = leaf;
            for (auto& f : br) c = mk(f, c, x->id, "", nm);
            tips.push_back(c);
        }
        for (int i = (int)tips.size() - 1; i >= 0; i--) stack.push_back(tips[i]);
    }
    for (auto& n : pool) if (n->from) applied.insert(n->from);
}

void Tableau::draw(Node* n, const string& head, const string& cont) const {
    cout << head << "[" << n->id << "] " << S(n->f) << (applied.count(n->id) ? " ✓" : "")
         << "   (" << (n->from ? "de " + to_string(n->from) + " · " + n->rule : n->tag) << ")\n";
    if (n->end == 1) cout << cont << "    ✕ rama cerrada: " << n->clash << "\n";
    if (n->end == 2) cout << cont << "    ● rama abierta\n";
    size_t k = n->kids.size();
    if (k == 1) draw(n->kids[0], cont, cont);
    else for (size_t i = 0; i < k; i++) {
        bool last = i + 1 == k;
        draw(n->kids[i], cont + (last ? "└─ " : "├─ "), cont + (last ? "   " : "│  "));
    }
}
