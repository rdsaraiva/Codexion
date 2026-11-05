/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 12:21:41 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/05 11:26:06 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (((long)tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	sleep_ms(long ms, t_sim *sim)
{
	long	target;

	target = now_ms() + ms;
	while (now_ms() < target && sim_running(sim))
		usleep(500);
}
