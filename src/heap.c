/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 14:00:16 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/09 12:00:00 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_init(t_heap *h, int capacity)
{
	h->items = malloc(sizeof(t_req) * capacity);
	if (h->items == NULL)
		return (1);
	h->size = 0;
	h->capacity = capacity;
	return (0);
}

void	heap_free(t_heap *h)
{
	free(h->items);
	h->items = NULL;
	h->size = 0;
}

int	heap_push(t_heap *h, t_req req)
{
	if (h->size >= h->capacity)
		return (1);
	h->items[h->size] = req;
	h->size += 1;
	sift_up(h, h->size - 1);
	return (0);
}

t_req	heap_pop(t_heap *h)
{
	t_req	top;

	top = h->items[0];
	h->size -= 1;
	h->items[0] = h->items[h->size];
	sift_down(h, 0);
	return (top);
}

void	heap_remove(t_heap *h, int id)
{
	int	i;

	i = 0;
	while (i < h->size && h->items[i].id != id)
		i++;
	if (i == h->size)
		return ;
	h->size -= 1;
	h->items[i] = h->items[h->size];
	sift_down(h, i);
	sift_up(h, i);
}
