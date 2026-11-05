/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 13:48:15 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/05 12:19:40 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongles(t_sim *sim)
{
	int			i;
	t_dongle	*d;

	i = 0;
	while (i < sim->p.coders)
	{
		d = &sim->dongles[i];
		pthread_mutex_init(&d->lock, NULL);
		pthread_cond_init(&d->cond, NULL);
		d->taken = 0;
		d->free_at = 0;
		d->ticket = 0;
		if (heap_init(&d->queue, 2))
			return (1);
		i++;
	}
	return (0);
}

static void	init_coders(t_sim *sim)
{
	int		i;
	int		a;
	int		b;
	t_coder	*c;

	i = 0;
	while (i < sim->p.coders)
	{
		c = &sim->coders[i];
		a = i;
		b = (i + 1) % sim->p.coders;
		c->compiles = 0;
		c->last_compile = sim->start;
		c->sim = sim;
		c->id = i + 1;
		c->first = &sim->dongles[a];
		c->second = &sim->dongles[b];
		if (c->id % 2 == 0)
		{
			c->first = &sim->dongles[b];
			c->second = &sim->dongles[a];
		}
		i++;
	}
}

int	sim_init(t_sim *sim)
{
	sim->stop = 0;
	pthread_mutex_init(&sim->state_lock, NULL);
	pthread_mutex_init(&sim->print_lock, NULL);
	sim->coders = malloc(sizeof(t_coder) * sim->p.coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->p.coders);
	if (sim->coders == NULL || sim->dongles == NULL)
		return (1);
	if (init_dongles(sim))
		return (1);
	sim->start = now_ms();
	init_coders(sim);
	return (0);
}
