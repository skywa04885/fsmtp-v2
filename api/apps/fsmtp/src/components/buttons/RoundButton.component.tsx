import React from 'react';

import './RoundButton.styles.scss';

interface RoundButtonProps
{
  onClick: (e: Event) => {},
  children: any
};

export default class RoundButton extends React.Component<any, any>
{
  public constructor(props: RoundButtonProps)
  {
    super(props);
  }

  public render(): any
  {
    const { children, onClick } = this.props;

    return (
      <button onClick={onClick} className="round-button">
        {children}
      </button>
    );
  }
}