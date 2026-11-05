/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   state.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 14:00:16 by roda-fon          #+#    #+#             */
/*   Updated: 2026/10/28 15:58:18 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	sim_running(t_sim *sim)
{
	int	stop;

	pthread_mutex_lock(&sim->state_lock);
	stop = sim->stop;
	pthread_mutex_unlock(&sim->state_lock);
	return (!stop);
}

void	sim_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->state_lock);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->state_lock);
}

void	log_state(t_sim *sim, int id, char *msg)
{
	pthread_mutex_lock(&sim->print_lock);
	if (sim_running(sim))
		printf("%ld %d %s\n", now_ms() - sim->start, id, msg);
	pthread_mutex_unlock(&sim->print_lock);
}
