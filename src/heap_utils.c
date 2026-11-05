/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 12:00:57 by roda-fon          #+#    #+#             */
/*   Updated: 2026/10/27 11:43:37 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	req_less(t_req a, t_req b)
{
	return (a.key < b.key || (a.key == b.key && a.id < b.id));
}

void	heap_swap(t_req *a, t_req *b)
{
	t_req	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	sift_up(t_heap *h, int i)
{
	int	parent;

	parent = (i - 1) / 2;
	while (i > 0 && req_less(h->items[i], h->items[parent]))
	{
		heap_swap(&h->items[i], &h->items[parent]);
		i = parent;
		parent = (i - 1) / 2;
	}
}

void	sift_down(t_heap *h, int i)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < h->size && req_less(h->items[left], h->items[smallest]))
			smallest = left;
		if (right < h->size && req_less(h->items[right], h->items[smallest]))
			smallest = right;
		if (smallest == i)
			break ;
		heap_swap(&h->items[i], &h->items[smallest]);
		i = smallest;
	}
}
