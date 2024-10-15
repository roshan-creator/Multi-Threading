import re

# Parse update log
updates = {}
with open('update.log') as f:
    for line in f:
        match = re.search('EDGE (\d+) (\d+) (ADDED|REMOVED)', line)
        if match:
            u = int(match.group(1))
            v = int(match.group(2))
            op = match.group(3)
            updates[(u,v)] = op

# Parse path found log            
found_paths = []
with open('path_found.log') as f:
    for line in f:
        if line.startswith('PATH_FOUND'):
            path = [int(x) for x in next(f).split()] 
            found_paths.append(path)
            
# Parse no path log
no_paths = []            
with open('path_not_found.log') as f:
    for line in f:
        if line.startswith('PATH_NOT_FOUND'):
            src, dest = [int(x) for x in line.split()[2:]]
            no_paths.append((src, dest))

# Check no paths
for src, dest in no_paths:
    if (src, dest) not in original_graph_edges:
        print(f"No path between {src} and {dest} as edge does not exist")
        continue
        
    edge_removed = False 
    for u, v in updates:
        if u in src_to_dest_shortest_path and v in src_to_dest_shortest_path:
            print(f"No path between {src} and {dest} due to edge ({u}, {v}) removed")
            edge_removed = True
            break
            
    if not edge_removed:
        print(f"Invalid no path between {src} and {dest}")
        
# Check new paths            
for path in found_paths:
    new_edge = False
    for u, v in zip(path[:-1], path[1:]):
        if (u, v) not in original_graph_edges and (u, v) in updates:
            print(f"Found new path {path} using new edge ({u}, {v})")
            new_edge = True
            break
            
    if not new_edge:
        print(f"Invalid new path found: {path}")