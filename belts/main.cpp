#include <bits/stdc++.h>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

const double INF = 1e18;
const double EPS = 1e-9;

struct Edge{
    string from, to;
    double lo, hi;
    int id;
};

class Dinic{
    public:
    struct edge{
        int to, rev;
        double lo, hi, flow, cap;
        int id;
    };

    int n, s, t, mxid;
    vector<int> d, done;
    vector<double> flow_through;
    vector<vector<edge>> g;

    Dinic() : n(0), s(0), t(0), mxid(-1) {}
    Dinic(int _n){
        n = _n;
        mxid = -1;
        g.assign(n, {});
    }

    void add_edge(int u, int v, double w, int id = -1){
        edge a = {v, (int)g[v].size(), 0, w, 0, w, id};
        edge b = {u, (int)g[u].size(), 0, 0, 0, 0, -2};
        g[u].push_back(a);
        g[v].push_back(b);
        mxid = max(mxid, id);
    }

    bool bfs(){
        d.assign(n, -1);
        queue<int> q;
        d[s] = 0;
        q.push(s);
        while (!q.empty()){
            int u = q.front();
            q.pop();
            for (auto &e : g[u]){
                int v = e.to;
                if (d[v] == -1 && e.cap - e.flow > EPS){
                    d[v] = d[u] + 1;
                    q.push(v);
                }
            }
        }
        return d[t] != -1;
    }

    double dfs(int u, double pushed){
        if (u == t)
            return pushed;
        for (int &i = done[u]; i < (int)g[u].size(); ++i){
            edge &e = g[u][i];
            if (e.cap - e.flow <= EPS) continue;
            int v = e.to;
            if (d[v] != d[u] + 1) continue;
            double tr = dfs(v, min(pushed, e.cap - e.flow));
            if (tr > EPS){
                e.flow += tr;
                if (e.rev >= 0 && e.rev < (int)g[v].size()){
                    g[v][e.rev].flow -= tr;
                }
                return tr;
            }
        }
        return 0.0;
    }

    double max_flow(int _s, int _t){
        s = _s;
        t = _t;
        double flow = 0.0;
        while (bfs()){
            done.assign(n, 0);
            while (true){
                double nw = dfs(s, INF);
                if (nw <= EPS)break;
                flow += nw;
            }
        }
        if (mxid >= 0){
            flow_through.assign(mxid + 1, 0.0);
            for (int u = 0; u < n; ++u){
                for (auto &e : g[u]){
                    if (e.id >= 0 && e.id < (int)flow_through.size())
                        flow_through[e.id] = e.flow;
                }
            }
        }
        else
            flow_through.clear();
        return flow;
    }
    
};

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(10);

    json j_in;
    cin >> j_in;

    vector<Edge> edges;
    map<string, double> sources, node_caps;
    string sink_node_name;
    double total_source_supply = 0.0;
    set<string> node_names;

    if (j_in.contains("sources")){
        for (auto &[name, supply] : j_in["sources"].items()){
            sources[name] = supply;
            node_names.insert(name);
            total_source_supply += sources[name];
        }
    }

    sink_node_name = j_in["sink"];
    node_names.insert(sink_node_name);

    if (j_in.contains("node_caps")){
        for (auto &[name, cap] : j_in["node_caps"].items()){
            node_caps[name] = cap;
            node_names.insert(name);
        }
    }

    int edge_id_counter = 0;
    if (j_in.contains("edges")){
        for (auto &edge_json : j_in["edges"]){
            Edge e;
            e.from = edge_json["from"];
            e.to = edge_json["to"];
            e.lo = edge_json["lo"];
            e.hi = edge_json["hi"];
            e.id = edge_id_counter++;
            edges.push_back(e);
            node_names.insert(e.from);
            node_names.insert(e.to);
        }
    }

    map<string, int> node_to_id;
    int id_counter = 0;
    for (const string &nm : node_names){
        node_to_id[nm + "_in"] = id_counter++;
        node_to_id[nm + "_out"] = id_counter++;
    }

    int super_source = id_counter++;
    int s_star = id_counter++;
    int t_star = id_counter++;

    Dinic ad(id_counter + 5); // +5 for buffer

    for (const string &nm : node_names){
        if (node_caps.find(nm) == node_caps.end() && sources.find(nm) == sources.end() && nm != sink_node_name)
            ad.add_edge(node_to_id[nm + "_in"], node_to_id[nm + "_out"], INF);
    }

    map<string, double> imbalance;
    double total_demand_for_lo = 0.0;

    for (auto &e : edges){
        imbalance[e.from] -= e.lo;
        imbalance[e.to] += e.lo;
        double cap = e.hi - e.lo;
        cap = max(0.0, cap);
        ad.add_edge(node_to_id[e.from + "_out"], node_to_id[e.to + "_in"], cap, e.id);
    }

    for (auto &[name, bal] : imbalance){
        if (bal > EPS){
            ad.add_edge(s_star, node_to_id[name + "_in"], bal);
            total_demand_for_lo += bal;
        }
        else if (bal < -EPS)
            ad.add_edge(node_to_id[name + "_out"], t_star, -bal);
    }

    for (auto &[name, cap] : node_caps)
        ad.add_edge(node_to_id[name + "_in"], node_to_id[name + "_out"], cap);

    for (auto &[name, supply] : sources){
        ad.add_edge(super_source, node_to_id[name + "_in"], supply);
        ad.add_edge(node_to_id[name + "_in"], node_to_id[name + "_out"], INF);
    }

    ad.add_edge(node_to_id[sink_node_name + "_in"], node_to_id[sink_node_name + "_out"], INF);
    ad.add_edge(node_to_id[sink_node_name + "_out"], super_source, INF);

    double feasibility_flow = ad.max_flow(s_star, t_star);
    json j_out;

    if (abs(feasibility_flow - total_demand_for_lo) > EPS){
        j_out["status"] = "infeasible";

        vector<char> reachable(ad.n, false);
        queue<int> q;
        q.push(s_star);
        reachable[s_star] = true;
        while (!q.empty()){
            int u = q.front();
            q.pop();
            for (auto &e : ad.g[u]){
                int v = e.to;
                if (!reachable[v] && e.cap - e.flow > EPS){
                    reachable[v] = true;
                    q.push(v);
                }
            }
        }

        json cut_arr = json::array();
        for (const string &nm : node_names){
            int in_id = node_to_id[nm + "_in"];
            int out_id = node_to_id[nm + "_out"];
            if ((in_id < ad.n && reachable[in_id]) || (out_id < ad.n && reachable[out_id])){
                cut_arr.push_back(nm);
            }
        }

        json tight_nodes = json::array();
        for (const string &nm : node_names){
            int in_id = node_to_id[nm + "_in"];
            int out_id = node_to_id[nm + "_out"];
            if (in_id < ad.n && out_id < ad.n && reachable[in_id] && !reachable[out_id]){
                tight_nodes.push_back(nm);
            }
        }

        json tight_edges = json::array();
        for (auto &e : edges){
            int from_out = node_to_id[e.from + "_out"];
            int to_in = node_to_id[e.to + "_in"];
            bool from_reach = (from_out < ad.n && reachable[from_out]);
            bool to_reach = (to_in < ad.n && reachable[to_in]);
            if (from_reach && !to_reach){
                double actual_transformed_flow = 0.0;
                if (e.id < (int)ad.flow_through.size()){
                    actual_transformed_flow = ad.flow_through[e.id];
                }
                double unmet = max(0.0, e.lo - actual_transformed_flow);
                if (unmet > EPS) {
                    tight_edges.push_back({{"from", e.from},{"to", e.to},{"flow_needed", unmet}});
                }
            }
        }

        j_out["cut_reachable"] = cut_arr;
        j_out["deficit"] = {
            {"demand_balance", total_demand_for_lo - feasibility_flow},
            {"tight_nodes", tight_nodes},
            {"tight_edges", tight_edges}
        };
    }
    else{
        for (auto &e : ad.g[s_star]){
            e.cap = 0;
            if (e.rev < (int)ad.g[e.to].size()){
                ad.g[e.to][e.rev].cap = 0;
            }
        }

        for (int u = 0; u < ad.n; ++u){
            for (auto &e : ad.g[u]){
                if (e.to == t_star){
                    e.cap = 0;
                    if (e.rev < (int)ad.g[e.to].size()){
                        ad.g[e.to][e.rev].cap = 0;
                    }
                }
            }
        }

        int sink_out_id = node_to_id[sink_node_name + "_out"];
        for (auto &e : ad.g[sink_out_id]){
            if (e.to == super_source){
                e.cap = 0;
                if (e.rev < (int)ad.g[e.to].size()){
                    ad.g[e.to][e.rev].cap = 0;
                }
            }
        }

        ad.max_flow(super_source, node_to_id[sink_node_name + "_in"]);

        double main_flow = 0.0;
        for (auto &e : ad.g[super_source])
            main_flow += e.flow;

        j_out["status"] = "ok";
        j_out["max_flow_per_min"] = main_flow;

        json flows_json = json::array();
        for (auto &e : edges){
            double transformed_flow = 0.0;
            if (e.id < (int)ad.flow_through.size()){
                transformed_flow = ad.flow_through[e.id];
            }
            double final_edge_flow = transformed_flow + e.lo;
            if (abs(final_edge_flow) < EPS){
                final_edge_flow = 0.0;
            }
            flows_json.push_back({{"from", e.from}, {"to", e.to}, {"flow", final_edge_flow}});
        }
        j_out["flows"] = flows_json;
    }

    cout << j_out.dump(4) << "\n";
    return 0;
}
