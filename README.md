### BELTS FLOW MODEL

## Assumptions made
- Input is a valid json file.
- Every node name is unique.
- All numeric values (capacities, supplies) are non-negative doubles.
- The sink node appears exactly once.
- Edge capacity lo ≤ hi always holds.
- The input json file has the following fields
    * "sources": list of sources and their supply
    * "sink": single sink node
    * "edges": list of edges having fields from, to, lo, hi
    * "node_caps": capacities for all the intermediate nodes


## Modeling Choices
# Max-Flow with Lower Bounds
- Covert the edge capacity to (hi - lo).
- Maintain the lower bound by adjusting node demands:
demand[v] += lo, demand[u] -= lo.
- Add a super-source S* and super-sink T*.
- Connect nodes with positive demand to T* and nodes with negative demand to S*.
- Feasibility check: the flow from S* to T* must equal total demand.

# Order of operations:
- Parse JSON and build node/edge lists.
- Apply lower-bound transformation.
- Add node capacity edges (via splitting).
- Add super-source/sink edges.
- Run Dinic’s algorithm.

# Node-Splitting for Capacity Constraints
Each node X is represented as:
X_in → X_out  (capacity = node_cap[X])
All incoming edges connect to X_in and outgoing edges start from X_out.

# Infeasibility Certificates (Min-Cut)
When Feasible:
- Restore the original graph with lower bounds and node splits.
- Run a standard max-flow from actual sources to sink to compute total achievable flow.

When Infeasible:
- The system violates one or more lower bounds, meaning:
    * Not enough supply from sources to fulfill the lo for all the edges.
    * A node or edge is too constrained.
    * Graph disconnected under required minimum flows.
- The reachable set from the super-source in the residual graph represents the cut.
- Nodes on the source side are those whose demands cannot be fully satisfied.
- This set is reported or logged as an infeasibility witness.

# NOTE:
A small part of the structure was refined using LLM for clarity and code formatting, but the core algorithmic logic, transformations, and feasibility modeling are fully my own work.

------------------------------------------------------------------------------------------------
### FACTORY STEADY STATE
- Not done