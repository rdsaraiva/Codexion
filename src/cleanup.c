/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 15:47:25 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/09 12:00:00 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	sim_destroy(t_sim *sim)
{
	int	i;

	i = 0;
	while (sim->dongles && i < sim->p.coders)
	{
		heap_free(&sim->dongles[i].queue);
		pthread_mutex_destroy(&sim->dongles[i].lock);
		i++;
	}
	free(sim->dongles);
	free(sim->coders);
	pthread_mutex_destroy(&sim->state_lock);
	pthread_mutex_destroy(&sim->print_lock);
	pthread_mutex_destroy(&sim->table_lock);
	pthread_cond_destroy(&sim->table_cond);
}
