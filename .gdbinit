define p_nodes
  set $i = 0
  set $limit = h->nodes_num
  while $i < $limit
    # Ваши команды GDB, которые будут выполняться в цикле
    printf "Iteration %d\n", $i

    # Пример: Печать значений массива h->nodes[$i]
    # printf "h->nodes[%d] = %d\n", $i, h->nodes[$i]
    p *h->nodes[$i]

    set $i = $i + 1
  end
end

define r
    !reset
    run
end


b 275
define I
    p h->nodes_num
    p_nodes
    p h->nodes_num
    echo "----------------------------\n"
    c
end

#r
